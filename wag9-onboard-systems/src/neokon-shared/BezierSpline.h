#pragma once

#include "BezierSegment.h"
#include "CalcOffsetedBezierSegment.h"

#include <UnigineMathLib.h>

#include <boost/icl/split_interval_map.hpp>

#include <optional>

/**
 * This class represents set of BezierSegments with their length intervals for easy spline
 * manipulations.
 *
 * Contains cubic Bezier curves(m_segments).
 */
class BezierSpline
{
public:
    using SegmentsIntervalMap = boost::icl::split_interval_map<double, std::shared_ptr<BezierSegment>>;

    struct PointToInsert
    {
        /**
         * Distance from spline start where the new point should be inserted
         */
        double distance;

        /**
         * Position override value for the new inserted point, tangent is calculated from the spline
         */
        glm::dvec3 position;
    };

    using ModifyPointFunc = std::function<void(double distance, glm::dvec3& position,
        glm::dvec3& backTangent, glm::dvec3& frontTangent)>;

public:
    BezierSpline() = default;
    BezierSpline(BezierSpline&& other);
    BezierSpline(const BezierSpline& other);
    BezierSpline(const BezierSegment& segment);
    BezierSpline(const std::vector<glm::dvec3>& points);

    BezierSpline& operator=(BezierSpline&& other);

    bool isDifferentWith(const BezierSpline& other, double epsilonSquared = 0.01) const;

    /**
     * Subdivide spline - insert points at 'distance' with predefined 'position' and tangents,
     * calculated from the spline
     */
    int insertPoints(const std::vector<PointToInsert>& points);

    /**
     * Subdivide spline - insert points at 'distances', calculate their position and tangents from
     * the spline and pass them to 'modifyFunc' call for modification
     */
    int insertPoints(const std::vector<double>& distances, const ModifyPointFunc& modifyFunc);

    double appendSegment(const BezierSegment& segment);
    double appendSpline(const BezierSpline& spline);
    const SegmentsIntervalMap& getSegmentsMap() const { return m_segments; }
    std::vector<BezierSegment> buildSegmentsVector() const;

    /**
     * Create new spline from this with 'offset' in local coord system along spline.
     */
    BezierSpline createOffseted(const glm::dvec3& offset) const;

    /**
     * Create new spline from this, trimmed by 'startLength' and 'endLength'.
     */
    BezierSpline createTrimmed(double startLength, std::optional<double> endLength = std::nullopt) const;

    /**
     * Create new spline from this, but with opposite direction.
     */
    BezierSpline createReversed() const;

    /**
     * Create new spline from this, localized to 'transform', if it is empty, it will be set to
     * transform, aligned to firts-to-last spline point line.
     */
    BezierSpline createLocalized(std::optional<Unigine::Math::Mat4>& transform) const;

    bool empty() const { return m_segments.empty(); }
    double arcLength() const { return m_length; }
    const AlignedBox<glm::dvec3>& getAABB() const;

    /**
     * Get translate of spline at 'distance'. Return std::nullopt in case out-of-bounds 'distance'.
     *
     * @param clampDistance - if passed true, then param @distance
     * will be clamped to bounds [0 + epsilon, arcLength() - epsilon] to garantee, that
     * result will not be 'std::nullopt'.
     */
    std::optional<glm::dvec3> pointAtDistance(double distance, bool clampDistance = false) const;

    /**
     * Get tangent of spline at 'distance'. Return std::nullopt in case out-of-bounds 'distance'.
     *
     * @param clampDistance - if passed true, then param @distance
     * will be clamped to bounds [0 + epsilon, arcLength() - epsilon] to garantee, that
     * result will not be 'std::nullopt'.
     */
    std::optional<glm::dvec3> unitTangentAtDistance(double distance, bool clampDistance = false) const;

    /**
     * Get transform, built from transform and tangent of spline at 'distance'.
     * Return std::nullopt in case out-of-bounds 'distance'.
     *
     * @param clampDistance - if passed true, then param @distance
     * will be clamped to bounds [0 + epsilon, arcLength() - epsilon] to garantee, that
     * result will not be 'std::nullopt'.
     */
    std::optional<Unigine::Math::Mat4> calcSplineMatrixAtDistance(double distance,
        bool clampDistance = false,
        TransformOrthogonalization orthogonalization = TransformOrthogonalization::On) const;
    std::optional<Unigine::Math::Mat4> startDistanceMatrix() const;
    std::optional<Unigine::Math::Mat4> endDistanceMatrix() const;

private:
    double clampDistanceToBounds(double srcDistance) const;

    double m_length = 0.0;
    SegmentsIntervalMap m_segments;
    std::optional<AlignedBox<glm::dvec3>> m_aabb;
};

bool operator==(BezierSpline const& left, BezierSpline const& right);
bool operator!=(BezierSpline const& left, BezierSpline const& right);
