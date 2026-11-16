#pragma once

#include "BezierSpline.h"

#include <UnigineMathLib.h>

#include <nanoflann.hpp>

#include <optional>

/**
 * This class is intended to be used to:
 *  - find points with prefered distance to spline.
 *  - represent spline of transforms, calculated from two bezier, i.e. spline in the middle of two
 *      splines.
 * Generally it a BVH of points, calculated from splines with fixed step, stored in segments interval
 * map. Accuracy of 'findClosestPoint'/'findClosestPoints' is increased by two stage search:
 *  - find nearest segment
 *  - calculate nearest point on segment
 */
class TransformSpline
{
public:
    using SegmentsIntervalMap = boost::icl::split_interval_map<double,
        std::shared_ptr<Unigine::Math::Mat4>>;

    struct Point
    {
        double distanceOffset = 0.0;
        Unigine::Math::Mat4 transform;
    };

    struct NeighbourPoints
    {
        double gap = 0.0;
        Point first;
        Point second;
    };

    using TestVectorFunc = std::function<void(const Unigine::Math::Vec3& from,
        const Unigine::Math::Vec3& to, const Unigine::Math::vec4& color)>;
    using CalcSplineMatrixAtDistanceFunc =
        std::function<std::optional<Unigine::Math::Mat4>(double distance)>;

public:
    TransformSpline();
    TransformSpline(const TransformSpline& other);

    bool isEmpty() const { return m_points.empty(); }

    /**
     * Build for single 'BezierSpline' with distance 'step'. If 'start' != 0., do not forget to add
     * it to find... results distances by yourself - as this TransformSpline's start distance is 0.
     * Negative step is supported.
     */
    double build(const BezierSpline& bezierSpline, double step = 0.1, double start = 0.,
        std::optional<double> end = std::nullopt);
    double build(const CalcSplineMatrixAtDistanceFunc& calcFunc, double step, double start,
        double end);

    /**
     * Build for single 'BezierSpline' with 'numSteps' by distance. If 'start' != 0., do not forget 
     * to add it to find... results distances by yourself - as this TransformSpline's start distance 
     * is 0. Negative step is supported.
     */
    double buildNumSteps(const BezierSpline& bezierSpline, int numSteps, double start = 0,
        std::optional<double> end = std::nullopt);
    double buildNumSteps(const CalcSplineMatrixAtDistanceFunc& calcFunc, int numSteps, double start,
        double end);

    /**
     * Build for two 'BezierSpline' with distance 'step' at offset 'k' between them
     * 'firstStartM' - start offset for first spline,
     * 'secondStartM' - start offset for second spline,
     * 'maxLengthM' - maximum length for both splines (std::nullopt = min from splines),
     * 'keepLength' - use full length of every spline (splineStep = splineLength / numSteps)
     * Negative step is not supported
     */
    double build(const BezierSpline& first, const BezierSpline& second,
        const std::function<double(double)>& k, double step = 0.1, double firstStartM = 0.0,
        double secondStartM = 0.0, std::optional<double> maxLengthM = std::nullopt,
        bool keepLength = false);
    void clear();

    /**
     * Get arc length of the spline.
     */
    double arcLength() const { return m_length; }

    /**
     * Get step points.
     */
    const std::vector<Point>& getPoints() const { return m_points; }

    /**
     * Get transform of spline at 'distance'. Return std::nullopt in case out-of-bounds 'distance'.
     */
    std::optional<Unigine::Math::Mat4> getTransformAtDistance(double distance) const;

    /**
     * Find point on spline with minimal distance to 'position'.
     * If found, returns 'Point' there Point::distance is set to distance offset on this
     * TransformSpline and 'transform' = transform of this point.
     */
    std::optional<Point> findClosestPoint(const glm::dvec3& position) const;

    /**
     * Find points on spline and line segment (from=>to) with minimal distance between them.
     * If found, returns 'Points' with '.distance' set to distance between result points,
     * 'first' set to point on this TransformSpline and 'second' to point on line segment,
     * there Point::distance is set to distance offset and 'transform' = transform of point.
     */
    std::optional<NeighbourPoints> findClosestPoints(const glm::dvec3& from, const glm::dvec3& to,
        double stepM = 0.1) const;
    std::optional<NeighbourPoints> findClosestPoints(const TransformSpline& other) const;

    /**
     * Find points on two splines with 'findDistance' between them.
     * If found, returns 'Points' with '.distance' set to distance between result points,
     * 'first' set to point on this TransformSpline and 'second' to point on 'second' BezierSpline,
     * there Point::distance is set to distance offset and 'transform' = transform of point.
     * WARNING: This functions search for shortest distance - perpendicular to this spline.
     */
    std::optional<NeighbourPoints> findPointsWithDistance(const BezierSpline& second, double findDistance,
        double stepM = 0.1, double startM = 0.0, std::optional<double> endM = std::nullopt) const;

    /**
     * Scan this spline with 'stepM' from 'startM' to 'endM' distance and measure distance,
     * projected to perpendicular plane, between 'from' and 'to' spline. Return nearest points with
     * distance, neares to 'findDistance'.
     */
    std::optional<NeighbourPoints> findPointsAcrossWithDistance(const TransformSpline& from,
        const TransformSpline& to, double findDistance, double stepM = 0.1, double startM = 0.0,
        std::optional<double> endM = std::nullopt) const;

public: // Debugging functions
    void setTestVectorFunc(const TestVectorFunc& func);
    void testVector(const Unigine::Math::Vec3& from, const Unigine::Math::Vec3& to,
        const Unigine::Math::vec4& color) const;

private:
    class PointsAdaptor
    {
    public:
        /// The constructor that sets the data set source
        PointsAdaptor(const std::vector<Point>& points) : m_points(points) { }

        /// CRTP helper method
        //inline const Derived& derived() const { return obj; }

        inline size_t kdtree_get_point_count() const { return m_points.size(); }
        inline double kdtree_get_pt(const size_t idx, int dim) const
        {
            if (dim == 0) return m_points[idx].transform.getTranslate().x;
            else if (dim == 1) return m_points[idx].transform.getTranslate().y;
            else return m_points[idx].transform.getTranslate().z;
        }
        template <class BBOX>
        bool kdtree_get_bbox(BBOX& /*bb*/) const { return false; }

    private:
        const std::vector<Point>& m_points;
    };

    using KdTreeType = nanoflann::KDTreeSingleIndexAdaptor<
        nanoflann::L2_Simple_Adaptor<double, PointsAdaptor>,
        PointsAdaptor,
        3,
        size_t>;

    double appendSegment(const Unigine::Math::Mat4& transfom, double length);

    double m_length = 0.0;

    std::vector<Point> m_points;
    SegmentsIntervalMap m_segments;

    PointsAdaptor m_pointsAdaptor;
    KdTreeType m_pointsTree;

#ifdef _DEBUG
    TestVectorFunc m_testVector;
#endif
};
