#pragma once

#include "neokon-shared/BezierSpline.h"
#include "neokon-shared/GenerateEnoughSplinePoints.h"
#include "neokon-shared/GlmUnigineInterop.h"
#include "neokon-shared/TransformSpline.h"

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/iterator/function_output_iterator.hpp>

#include <shared_mutex>

namespace bg = boost::geometry;
namespace bgm = boost::geometry::model;
namespace bgi = boost::geometry::index;

/**
 * Stores multiple splines for searching purposes. 'MappedType' can be any type to be assigned to
 * added segments, but it must have hashing support.
 */
template<typename MappedType>
class SplinesMap : public std::enable_shared_from_this<SplinesMap<MappedType>>
{
public:
    using PointType = bgm::d2::point_xy<double, bg::cs::cartesian>;
    using BoxType = bgm::box<PointType>;

public:
    /**
     * Wrapper, required by bgi::rtree for placing weak_ptr into it.
     */
    template<typename Type>
    class WeakPtrCompareWrapper
    {
    public:
        WeakPtrCompareWrapper(const std::weak_ptr<Type>& value)
            : m_value(value)
        {
        }

        WeakPtrCompareWrapper(const WeakPtrCompareWrapper<Type>& other)
            : m_value(other.m_value)
        {
        }

        bool operator==(const WeakPtrCompareWrapper<Type>& other) const 
        { 
            if (m_value.expired())
            {
                return other.m_value.expired();
            }

            if (other.m_value.expired())
            {
                return m_value.expired();
            }

            return m_value.lock() == other.m_value.lock();
        }

        operator const std::weak_ptr<Type>& () const 
        {
            return m_value;
        }

    private:
        std::weak_ptr<Type> m_value;
    };

    /**
     * Class, representing single BezierSegment.
     */
    class Segment : public std::enable_shared_from_this<Segment>
    {
    public:
        static std::shared_ptr<Segment> create(const std::weak_ptr<SplinesMap>& parentMap,
            const BezierSegment& parentSegment, double t0, double t1, double splineOffset,
            double tangentialOffsetSquared, const MappedType& mapped)
        {
            std::shared_ptr<Segment> result(new Segment(parentMap, parentSegment, t0, t1,
                splineOffset, tangentialOffsetSquared, mapped));
            result->init();
            return result;
        }

        Segment(const std::weak_ptr<SplinesMap>& parentMap, const BezierSegment& parentSegment,
            double t0, double t1, double splineOffset, double tangentialOffsetSquared,
            const MappedType& mapped)
            : m_parentMap(parentMap)
            , m_parentSegment(parentSegment)
            , m_splineOffset(splineOffset)
            , m_tangentialOffsetSquared(tangentialOffsetSquared)
            , m_mapped(mapped)
        {
            auto dst1 = m_parentSegment.arcLengthFromTo(0.0, t0);
            auto dst2 = m_parentSegment.arcLengthFromTo(0.0, t1);

            m_subSegment = m_parentSegment.subSegmentDistanceToDistance(dst1, dst2);

            m_parentSegmentOffset = dst1;
        }

        const MappedType& getMapped() const { return m_mapped; }
        double getSplineOffset() const { return m_splineOffset + m_parentSegmentOffset; }
        double getParentSegmentOffset() const { return m_parentSegmentOffset; }

        std::optional<TransformSpline::Point> findClosestPoint(const glm::dvec3& point) 
        { 
            if (m_splineSearch.isEmpty())
            {
                // TODO: Make this dependent of 'm_tangentialOffsetSquared' and length, because
                // very long segments with non-zero 'm_tangentialOffsetSquared' can have huge errors
                // http://git.neokon.local:22580/root/GlobalTerrainEditor/-/issues/940
                int numSteps = 1;
                m_splineSearch.buildNumSteps(BezierSpline(m_subSegment), numSteps);
            }

            return m_splineSearch.findClosestPoint(point);
        }

        double parentParamAtDistance(double distance)
        {
            return m_parentSegment.tAtDistance(distance);
        }

        virtual ~Segment() 
        { 
            if (m_partPair)
            {
                if (auto parentMap = m_parentMap.lock())
                {
                    std::unique_lock lock(parentMap->m_rtreeProtect);
                    parentMap->m_rtree.remove(*m_partPair);
                }
            }
        }

    private:
        void init()
        {
            auto segmentBBox = m_subSegment.getAABB();
            BoxType bbox(PointType(segmentBBox.minCoords().x, segmentBBox.minCoords().y),
                PointType(segmentBBox.maxCoords().x, segmentBBox.maxCoords().y));

            m_partPair = std::make_pair(bbox, WeakPtrCompareWrapper<Segment>(weak_from_this()));

            if (auto parentMap = m_parentMap.lock())
            {
                std::unique_lock lock(parentMap->m_rtreeProtect);
                parentMap->m_rtree.insert(*m_partPair);
            }
        }

    private:
        MappedType m_mapped;
        std::weak_ptr<SplinesMap> m_parentMap;
        BezierSegment m_parentSegment;
        BezierSegment m_subSegment;
        double m_splineOffset{0.0};
        double m_parentSegmentOffset{0.0};
        double m_tangentialOffsetSquared{0.0};

        std::optional<std::pair<BoxType, WeakPtrCompareWrapper<Segment>>> m_partPair;
        TransformSpline m_splineSearch;
    };

    /**
     * Class, representing group of Bezier segments.
     */
    class Spline : public std::enable_shared_from_this<Spline>
    {
    public:
        Spline(const std::weak_ptr<SplinesMap>& parentMap)
            : m_parentMap(parentMap)
        {
        }

        Spline(const std::weak_ptr<SplinesMap>& parentMap,
            BezierSpline&& spline,
            const MappedType& mapped)
            : m_parentMap(parentMap)
        {
            double splineOffset = 0.0;
            for (const auto& segmentPair : spline.getSegmentsMap())
            {
                splineOffset = addSegment(splineOffset, *segmentPair.second, mapped);
            }
        }

        /**
         * Append BezierSegment, splitting it on parts with low tangential offset
         * 'splineOffset' - start distance relative to spline,
         * 'segment' - spline segment to append,
         * 'mapped' - value to be placed in every segment part
         * return next segment's 'splineOffset' == current spline length
         */
        double addSegment(double splineOffset, const BezierSegment& segment, const MappedType& mapped)
        {
            auto calcSplinePoint = [&segment](double param)
            {
                glm::dvec3 glmPosition = segment.eval(param);
                return fromGlm(glmPosition);
            };

            std::vector<float> tParams;
            Unigine::Vector<Unigine::Math::Vec3> points;
            std::vector<double> tangentialOffsetsSquared;
            tParams.emplace_back(0.0f);
            points.emplace_back(calcSplinePoint(0.0));
            tangentialOffsetsSquared.push_back(0.0);
            generateEnoughSplinePoints(calcSplinePoint, points, tParams,
                tangentialOffsetsSquared, 0.0f, 0.5f);
            generateEnoughSplinePoints(calcSplinePoint, points, tParams,
                tangentialOffsetsSquared, 0.5f, 1.0f);

            for (int partIdx = 1; partIdx < tParams.size(); ++partIdx)
            {
                const auto& tParam1 = tParams[partIdx - 1];
                const auto& tParam2 = tParams[partIdx];
                double tangentialOffsetSquared = tangentialOffsetsSquared[partIdx];

                m_segments.emplace_back(
                    Segment::create(m_parentMap, segment, tParam1, tParam2, splineOffset,
                    tangentialOffsetSquared, mapped));
            }

            m_spline.appendSegment(segment);
            return splineOffset;
        }

        virtual ~Spline() = default;

        const BezierSpline& getBezierSpline() const { return m_spline; }

        const std::vector<std::shared_ptr<Segment>>& getSegments() const { return m_segments; };

    private : 
        std::weak_ptr<SplinesMap> m_parentMap;
        BezierSpline m_spline;

        std::vector<std::shared_ptr<Segment>> m_segments;
    };

    using SplinePtr = std::shared_ptr<Spline>;
    using SplineWeak = std::weak_ptr<Spline>;

    using PartPair = std::pair<BoxType, WeakPtrCompareWrapper<Segment>>;

    /**
     * Wrapper, required by bgi::rtree::query for geathering results.
     */
    class PartInsertIterator
    {
    public:
        inline explicit PartInsertIterator(
            std::vector<std::weak_ptr<Segment>>& container)
            : m_container(container)
        {
        }

        PartInsertIterator& operator=(typename PartPair const& value)
        {
            m_container.emplace_back(value.second);
            return *this;
        }

        PartInsertIterator& operator*() { return *this; } // Not used by .query()

        PartInsertIterator& operator++() { return *this; } // Not used by .query()

        PartInsertIterator operator++(int) { return *this; } // Not used by .query()

    private:
        std::vector<std::weak_ptr<Segment>>& m_container;
    };

public:
    static std::shared_ptr<SplinesMap<MappedType>> create()
    {
        return std::shared_ptr<SplinesMap<MappedType>>(new SplinesMap<MappedType>());
    }

    ~SplinesMap() = default;

    std::vector<SplineWeak> getSplines() const
    {
        std::shared_lock lock(m_splinesProtect);
        return m_splines;
    }

    /**
     * Function for adding new spline, filled with segments from BezierSpline&&, all segments
     * mapped to single 'mapped'.
     */
    SplinePtr addSpline(BezierSpline&& spline, const MappedType& mapped) 
    {
        SplinePtr result(new Spline(weak_from_this(), std::move(spline), mapped));
        {
            std::unique_lock lock(m_splinesProtect);
            m_splines.emplace_back(result);
        }
        return result;
    }

    /**
     * Function for adding new empty spline, to fill it with segments manually by spline->addSegment()
     * and map every segment to different 'mapped'.
     */
    SplinePtr addSpline()
    {
        SplinePtr result(new Spline(weak_from_this()));
        {
            std::unique_lock lock(m_splinesProtect);
            m_splines.emplace_back(result);
        }
        return result;
    }

    struct PointOnSegment
    {
        MappedType mapped;
        double distance = 0; // distance is in [0..length]
        double param = 0;    // param is in [0..1]
        Unigine::Math::Vec3 worldPoint;
    };

    /**
     * Function for searching nearest segment and segment's point in rect 'extent' area
     * around 'centerPosition'. 'checkExclude' will be called for every mapped value to be excluded.
     */
    std::optional<PointOnSegment> findNearestSegmentPointWithinQuad(
        const Unigine::Math::Vec3& centerPosition, double extent,
        const std::function<bool(MappedType)>& checkExclude = {}) const
    {
        BoxType box(PointType(centerPosition.x - extent, centerPosition.y - extent),
            PointType(centerPosition.x + extent, centerPosition.y + extent));

        std::vector<std::weak_ptr<Segment>> segmentWeaks;
        {
            std::shared_lock lock(m_rtreeProtect);
            m_rtree.query(bgi::intersects(box),
                PartInsertIterator(segmentWeaks));
        }

        std::optional<PointOnSegment> result;
        for (const auto& segmentWeak : segmentWeaks)
        {
            if (auto segment = segmentWeak.lock())
            {
                if (auto closest = segment->findClosestPoint(toGlm(centerPosition)))
                {
                    auto translation = closest->transform.getTranslate();

                    const auto& mapped = segment->getMapped();
                    if (checkExclude && checkExclude(mapped))
                    {
                        continue;
                    }

                    if (!result ||
                        ((result->worldPoint - centerPosition).length2() >
                            (translation - centerPosition).length2()))
                    {
                        const double splineOffset =
                            closest->distanceOffset + segment->getSplineOffset();
                        const double param = segment->parentParamAtDistance(
                            closest->distanceOffset + segment->getParentSegmentOffset());

                        result = PointOnSegment{mapped, splineOffset, param, translation};
                    }
                }
            }
        }

        return result;
    }

    /**
     * Function for searching all segments in 'queryAABB' box area. Calls 'proc' for "mapped" value
     * to build result set from it. 'MappedProduct' type have to be returned by 'proc' to be added
     * to result set. 'MappedProduct' must have 'operator <' or should there be a specialization
     * for std::less.
     */
    template<typename MappedProduct>
    std::set<MappedProduct> queryMappedInAABB(const BoxType& queryAABB,
        const std::function<MappedProduct(const MappedType&)>& proc) const 
    {
        std::vector<std::weak_ptr<Segment>> segmentWeaks;
        {
            std::shared_lock lock(m_rtreeProtect);
            m_rtree.query(bgi::intersects(queryAABB),
                PartInsertIterator(segmentWeaks));
        }

        std::set<MappedProduct> result;
        for (const auto& segmentWeak : segmentWeaks)
        {
            if (auto segment = segmentWeak.lock())
            {
                result.insert(proc(segment->getMapped()));
            }
        }
        return result;
    }

    /**
     * Function for thread safe iteration of all 'm_rtree' boxes. For debugging mostly.
     */
    void forEachPart(const std::function<void(const BoxType&)>& proc)
    {
        std::shared_lock lock(m_rtreeProtect);
        for (const auto& pair : m_rtree)
        {
            proc(pair.first);
        }
    }

private:
    SplinesMap() {}

    mutable std::shared_mutex m_rtreeProtect;
    bgi::rtree<PartPair, bgi::quadratic<16>> m_rtree;

    mutable std::shared_mutex m_splinesProtect;
    std::vector<SplineWeak> m_splines;
};
