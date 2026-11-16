#include "BezierSegment.h"

#include "BezierArcLengthCalculator.h"

#include "neokon-shared/ProtobufGlmInterop.h"

#include "glm/gtx/norm.hpp"

#include <cassert>
#include <cmath>
#include <cstring>
#include <vector>
#include <utility>
#include <cmath>

namespace
{

glm::ivec3 greaterThan(const glm::dvec3& vec, const glm::dvec3& other)
{
    return glm::ivec3(vec.x > other.x ? 1 : 0,
        vec.y > other.y ? 1 : 0,
        vec.z > other.z ? 1 : 0);
}

bool any(const glm::ivec3& vec)
{
    return (vec.x > 0) || (vec.y > 0) || (vec.z > 0);
}

glm::dvec3 clamp(const glm::dvec3& vec, double min, double max)
{
    return glm::dvec3(std::clamp(vec.x, min, max),
        std::clamp(vec.y, min, max),
        std::clamp(vec.z, min, max));
}

}// anonymous namespace

BezierSegment::BezierSegment(
    glm::dvec3 const& p0, glm::dvec3 const& p1,
    glm::dvec3 const& p2, glm::dvec3 const& p3)
    : m_mat(p0, p1, p2, p3)
{
    calcAABB();
}

BezierSegment::BezierSegment(proto::BezierSegment const& protoSeg)
    : m_mat(
        protoToGlm(protoSeg.p0()), protoToGlm(protoSeg.p1()),
        protoToGlm(protoSeg.p2()), protoToGlm(protoSeg.p3()))
{
    calcAABB();
}

bool BezierSegment::isDifferentWith(const BezierSegment& other, double epsilonSquared) const
{
    if (glm::distance2(p0(), other.p0()) > epsilonSquared)
    {
        return true;
    }
    if (glm::distance2(p1(), other.p1()) > epsilonSquared)
    {
        return true;
    }
    if (glm::distance2(p2(), other.p2()) > epsilonSquared)
    {
        return true;
    }
    if (glm::distance2(p3(), other.p3()) > epsilonSquared)
    {
        return true;
    }
    return false;
}

bool
BezierSegment::isStraight() const
{
    return glm::dot(arcLengthBounds(), glm::dvec2(-1.0, 1.0)) < 1e-4;
}

double
BezierSegment::chordLength() const
{
    return glm::distance(p0(), p3());
}

double
BezierSegment::controlPolygonLength() const
{
    return glm::distance(p0(), p1()) +
           glm::distance(p1(), p2()) +
           glm::distance(p2(), p3());
}

glm::dvec2
BezierSegment::arcLengthBounds() const
{
    return glm::dvec2(chordLength(), controlPolygonLength());
}

BezierSegment
BezierSegment::reversed() const
{
    return BezierSegment(m_mat[3], m_mat[2], m_mat[1], m_mat[0]);
}

glm::dvec3
BezierSegment::eval(double const t) const
{
    double const t1 = 1.0 - t;

    return m_mat * glm::dvec4(
        t1 * t1 * t1,
        3.0 * (t1 * t1) * t,
        3.0 * (t * t) * t1,
        t * t * t
    );
}

glm::dvec3
BezierSegment::evalDeriv(double t) const
{
    double const t1 = 1.0 - t;

    // d(t)/dt = 1
    // d(t1)/dt = -1
    // d(t * t)/dt = 2 * t
    // d(t1 * t1)/dt = -2 * t1
    // d(t1 * t1 * t1)/dt = -3 * t1 * t1
    // d(3 * t1 * t1 * t)/dt = 3 * (d(t1 * t1)/dt * t + t1 * t1) = 3 * (-2 * t1 * t + t1 * t1)
    // d(3 * t * t * t1)/dt = 3 * (d(t * t)/dt * t1 - t * t) = 3 * (2 * t * t1 - t * t)
    // d(t * t * t)/dt = 3 * t * t

    return m_mat * glm::dvec4(
        -3.0 * (t1 * t1),
        -6.0 * (t * t1) + 3.0 * (t1 * t1),
        6.0 * (t * t1) - 3.0 * (t * t),
        3.0 * (t * t)
    );
}

glm::dvec3
BezierSegment::unitTangentAt(double t) const
{
    return glm::normalize(evalDeriv(t));
}

std::array<BezierSegment, 2>
BezierSegment::subdivide(double const t) const
{
    // See http://algorithmist.net/docs/subdivision.pdf

    glm::dvec3 const p11 = glm::mix(p0(), p1(), t);
    glm::dvec3 const p21 = glm::mix(p1(), p2(), t);
    glm::dvec3 const p31 = glm::mix(p2(), p3(), t);
    glm::dvec3 const p12 = glm::mix(p11, p21, t);
    glm::dvec3 const p22 = glm::mix(p21, p31, t);
    glm::dvec3 const p13 = glm::mix(p12, p22, t);

    return std::array<BezierSegment, 2>{
        BezierSegment(p0(), p11, p12, p13),
        BezierSegment(p13, p22, p31, p3())
    };
}

BezierSegment
BezierSegment::subSegmentDistanceToDistance(double fromDistance, double toDistance) const
{
    assert(std::isfinite(fromDistance));
    assert(std::isfinite(toDistance));

    if (fromDistance > toDistance)
    {
        return subSegmentDistanceToDistance(toDistance, fromDistance).reversed();
    }

    fromDistance = std::max<double>(0.0, fromDistance);
    toDistance = std::max<double>(fromDistance, toDistance);

    // toDistance and even fromDistance may exceed arcLength(), but that's fine, 
    // as tAtDistance() never returns results outside of the [0, 1] interval.
    // That effectively clamps both fromDistance and toDistance to arcLength().

    double const midSectionLength = toDistance - fromDistance;

    auto const noFrontSection = subdivide(tAtDistance(fromDistance)).back();
    return noFrontSection.subdivide(noFrontSection.tAtDistance(midSectionLength)).front();
}

double
BezierSegment::arcLength() const
{
    return arcLengthFromTo(0.0, 1.0);
}

double
BezierSegment::arcLengthFromTo(double const tStart, double const tEnd) const
{
    return BezierArcLengthCalculator(*this).arcLengthFromTo(tStart, tEnd);
}

glm::dvec3
BezierSegment::pointAtDistance(double const distance) const
{
    return eval(tAtDistance(distance));
}

double
BezierSegment::tAtDistance(double distance) const
{
    return BezierArcLengthCalculator(*this).tForDistance(distance);
}

BezierSegment
BezierSegment::softPulled(glm::dvec3 const& pullVec,
    double const p0Weight, double const p3Weight) const
{
    double const dist01 = glm::distance(p0(), p1());
    double const dist13 = glm::distance(p1(), p3());
    double const dist02 = glm::distance(p0(), p2());
    double const dist23 = glm::distance(p2(), p3());
    double const dist013 = dist01 + dist13;
    double const dist023 = dist02 + dist23;

    glm::dvec4 weights;
    weights[0] = p0Weight;
    weights[3] = p3Weight;

    if (dist013 < 1e-4f)
    {
        // This means p0, p1 and p3 are very close to each other.
        weights[1] = 0.5f * (p0Weight + p3Weight);
    }
    else
    {
        weights[1] = (dist13 / dist013) * p0Weight + (dist01 / dist013) * p3Weight;
    }

    if (dist023 < 1e-4f)
    {
        // This means p0, p2 and p3 are very close to each other.
        weights[2] = 0.5f * (p0Weight + p3Weight);
    }
    else
    {
        weights[2] = (dist23 / dist023) * p0Weight + (dist02 / dist023) * p3Weight;
    }

    glm::dmat4x3 newMat;
    for (int i = 0; i < 4; ++i)
    {
        newMat[i] = m_mat[i] + pullVec * weights[i];
    }

    return BezierSegment(newMat[0], newMat[1], newMat[2], newMat[3]);
}

void
BezierSegment::toXml(boost::property_tree::ptree& railSegmentNode) const
{
    namespace pt = boost::property_tree;

    char buf[256];

    auto format = [&buf](glm::dvec3 const& v)
    {
        // "%.17g" preserves all the digits in "double".
        // https://stackoverflow.com/questions/16839658

        snprintf(buf, sizeof(buf), "%.17g %.17g %.17g", v.x, v.y, v.z);
        return buf;
    };

    railSegmentNode.put("start_point", format(m_mat[0]));
    railSegmentNode.put("end_point", format(m_mat[3]));
    railSegmentNode.put("start_tangent", format(m_mat[1] - m_mat[0]));

    if (!isStraight())
    {
        railSegmentNode.put("end_tangent", format(m_mat[2] - m_mat[3]));
    }
}

proto::BezierSegment
BezierSegment::toProto() const
{
    proto::BezierSegment protoSeg;

    *protoSeg.mutable_p0() = glmToProto(m_mat[0]);
    *protoSeg.mutable_p1() = glmToProto(m_mat[1]);
    *protoSeg.mutable_p2() = glmToProto(m_mat[2]);
    *protoSeg.mutable_p3() = glmToProto(m_mat[3]);

    return protoSeg;
}

double
BezierSegment::subdivideRecursive(
    double maxDistanceBetweenPoints,
    double fromStartDistance,
    double fromT,
    double toT,
    std::function<void(
        double t,
        double arcLengthDistance)>const& sink) const
{
    double dist = arcLengthFromTo(fromT, toT);

    if (dist >= maxDistanceBetweenPoints)
    {
        double const midT = 0.5 * fromT + 0.5 * toT;

        double const midDist = fromStartDistance + subdivideRecursive(
            maxDistanceBetweenPoints, fromStartDistance, fromT, midT, sink);

        sink(midT, midDist);

        subdivideRecursive(maxDistanceBetweenPoints, midDist, midT, toT, sink);
    }

    return dist;
}

/**
 * This code is taken from here: https://iquilezles.org/articles/bezierbbox/
 *
 * The short explanation from the article:
 * The cubic Bezier spline equation is:
 * p(t) = (1-t)^3 * p0 + 3 * (1-t)^2 * t * p1 + 3 * (1-t) * t^2 * p2 + t^3 * p3
 * To find minimum and maximum extents of the curve we have to find such t where the function
 * derivative is zero and 0 <= t <= 1, or, for simplicity, p'(t) / 3:
 * p'(t)/3 = (1-t)^2 * (p1 - p0) + 2 * (1-t) * t * (p2-p1) + t2 * (p3-p2) = 0
 * To find t from the equation we consider it as a quadratic equation in form:
 * a * t^2 + 2 * b * t + c = 0
 * with a, b and c defined as vectors (see c,b,a coefficients assignment in the method).
 * @note that all arithmetic operations related to these coefficients are per-component.
 * Then we select an appropriate equation solution for t, put it into the original spline
 * equation and modify the original AABB calculated from the start and end points.
 */
void BezierSegment::calcAABB()
{
    // extremes
    glm::dvec3 mi = min(p0(), p3());
    glm::dvec3 ma = max(p0(), p3());

    // note pascal triangle coefficients
    const glm::dvec3 c = -1.0 * p0() + 1.0 * p1();
    const glm::dvec3 b = 1.0 * p0() - 2.0 * p1() + 1.0 * p2();
    const glm::dvec3 a = -1.0 * p0() + 3.0 * p1() - 3.0 * p2() + 1.0 * p3();

    const glm::dvec3 h = b * b - a * c;

    // real solutions
    if (any(greaterThan(h, glm::dvec3())))
    {
        const glm::dvec3 g = sqrt(abs(h));
        const glm::dvec3 t1 = clamp((-b - g) / a, 0.0, 1.0);
        const glm::dvec3 s1 = 1.0 - t1;
        const glm::dvec3 t2 = clamp((-b + g) / a, 0.0, 1.0);
        const glm::dvec3 s2 = 1.0 - t2;
        const glm::dvec3 q1 = s1 * s1 * s1 * p0() + 3.0 * s1 * s1 * t1 * p1() +
            3.0 * s1 * t1 * t1 * p2() + t1 * t1 * t1 * p3();
        const glm::dvec3 q2 = s2 * s2 * s2 * p0() + 3.0 * s2 * s2 * t2 * p1() +
            3.0 * s2 * t2 * t2 * p2() + t2 * t2 * t2 * p3();

        if (h.x > 0.0)
        {
            mi.x = std::min(mi.x, std::min(q1.x, q2.x));
            ma.x = std::max(ma.x, std::max(q1.x, q2.x));
        }
        if (h.y > 0.0)
        {
            mi.y = std::min(mi.y, std::min(q1.y, q2.y));
            ma.y = std::max(ma.y, std::max(q1.y, q2.y));
        }
        if (h.z > 0.0)
        {
            mi.z = std::min(mi.z, std::min(q1.z, q2.z));
            ma.z = std::max(ma.z, std::max(q1.z, q2.z));
        }
    }

    m_aabb = AlignedBox<glm::dvec3>(mi, ma);
}
