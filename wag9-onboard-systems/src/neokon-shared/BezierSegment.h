#pragma once

#include "BezierSegment.pb.h"

#include <boost/property_tree/ptree.hpp>
#include <glm/glm.hpp>

#include <array>

#include "AlignedBox.h"

/**
 * Cubic bezier curve.
 */
class BezierSegment
{
public:
    /**
     * Used in GlobalTerrainEditor, do not remove.
     */
    BezierSegment() = default;

    BezierSegment(
        glm::dvec3 const& p0, glm::dvec3 const& p1,
        glm::dvec3 const& p2, glm::dvec3 const& p3);

    explicit BezierSegment(proto::BezierSegment const& protoSeg);

    bool isDifferentWith(const BezierSegment& other, double epsilonSquared = 0.01) const;

    glm::dvec3 const& p0() const { return m_mat[0]; }

    glm::dvec3 const& p1() const { return m_mat[1]; }

    glm::dvec3 const& p2() const { return m_mat[2]; }

    glm::dvec3 const& p3() const { return m_mat[3]; }

    bool isStraight() const;

    double chordLength() const;

    double controlPolygonLength() const;

    /**
     * Returns glm::dvec2(chordLength(), controlPolygonLength()).
     */
    glm::dvec2 arcLengthBounds() const;

    BezierSegment reversed() const;

    /**
     * Returns a point on a curve at a specified t, ranging from 0 to 1.
     */
    glm::dvec3 eval(double t) const;

    /**
     * Returns the vector of first derivatives of the position(t) function, evaluated at t.
     */
    glm::dvec3 evalDeriv(double t) const;

    /**
     * Returns glm::normalize(evalDeriv(t)).
     */
    glm::dvec3 unitTangentAt(double t) const;

    /**
     * Subdivides the curve into front and back parts, where
     * frontPart.p3() == backPart.p0() == eval(t).
     *
     * Returns std::array<BezierSegment, 2>{ frontPart, backPart }.
     */
    std::array<BezierSegment, 2> subdivide(double t) const;

    /**
     * Extracts a sub-segment starting at @p fromDistance and ending at @p toDistance.
     *
     * The distance values passed here are effectively clamped to the [0, arcLength()]
     * interval. Also, @p fromDistance may be greater than @p toDistance. In this case,
     * the sub-segment's direction will be the opposite of the whole segment's direction.
     */
    BezierSegment subSegmentDistanceToDistance(double fromDistance, double toDistance) const;

    /**
     * Calculates an approximate arc length of this segment.
     */
    double arcLength() const;

    /**
     * Calculates an approximate arc length of a portion of this segment.
     *
     * tStart and tEnd must satisfy the following condition:
     * @code
     * 0 <= tStart <= tEnd <= 1
     * @endcode
     */
    double arcLengthFromTo(double tStart, double tEnd) const;

    /**
     * Returns eval(tAtDistance(distance)).
     */
    glm::dvec3 pointAtDistance(double distance) const;

    /**
     * Calculates the approximate value of parameter t that corresponds to the specified
     * arc length distance from the begining of the curve.
     *
     * The returned value is clamped to the range of [0, 1].
     */
    double tAtDistance(double distance) const;

    /**
     * We want to produce a new BezierSegment with "new_pX = pX + pullVec * pXWeight".
     *
     * p0Weight and p3Weight are given, and it's up to this method to come up with
     * p1Weight and p2Weight.
     */
    BezierSegment softPulled(glm::dvec3 const& pullVec, double p0Weight, double p3Weight) const;

    /**
     * Generates either a "RailSegment" or "CurvedRailSegment" node, depending on
     * what isStraight() returns.
     */
    void toXml(boost::property_tree::ptree& railSegmentNode) const;

    /**
     * Converts this BezierSegment to proto::BezierSegment.
     */
    proto::BezierSegment toProto() const;

    /**
     * Emumerates points on interval of this segment
     * by calling sink callback for each point.
     * Intreval start at fromT and ends at toT.
     *
     * It generates points on middle of interval and than repeats it recursively while
     * new interval length greater than maxDistanceBetweenPoints.
     * So bounds of interval are not included to enumeration.
     *
     * fromStartDistance is distance from segment start to fromT
     * Method returns arc length of the interval.
     */
    double subdivideRecursive(
        double maxDistanceBetweenPoints,
        double fromStartDistance,
        double fromT,
        double toT,
        std::function<void(
            double t,
            double arcLengthDistance)>const & sink) const;

    const AlignedBox<glm::dvec3>& getAABB() const { return m_aabb; }

private:
    void calcAABB();

    /**
     * That's actually a 3-row, 4-column matrix. The GLM naming is inconsistent.
     * Points p0 ... p3 are its columns.
     */
    glm::dmat4x3 m_mat;

    AlignedBox<glm::dvec3> m_aabb;
};
