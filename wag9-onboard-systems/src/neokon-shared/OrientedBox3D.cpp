#include "OrientedBox3D.h"

#include <cassert>
#include <cmath>

/**
 * The implementation is based on the following paper:
 * http://jkh.me/files/tutorials/Separating%20Axis%20Theorem%20for%20Oriented%20Bounding%20Boxes.pdf
 */
class OrientedBox3D::Intersector
{
public:
    Intersector(OrientedBox3D const& lhs, OrientedBox3D const& rhs);

    bool haveIntersection() const;

private:
    // Note that conditional moves are faster than the modulo operator.
    static int next(int i) { return i == 2 ? 0 : i + 1; }

    static int prev(int i) { return i == 0 ? 2 : i - 1; }

    bool separableOnAiAxis(int i) const;

    bool separableOnBjAxis(int j) const;

    bool separableOnAiCrossBjAxis(int i, int j) const;

    /**
     * transpose(rhs.m_axes) * lhs.m_axes
     *
     * That way, m_R[i][j] == dot(lhs.m_axes[i], rhs.m_axes[j]).
     * Note that in GLM, m[i][j] means the ith column and jth row, unlike
     * in Linear Algebra textbooks.
     * 
     */
    glm::dmat3 m_R;

    glm::dmat3 m_absR; // Element-wise absolute version of m_R.

    glm::dvec3 m_heA; // lhs.m_halfExtents
    glm::dvec3 m_heB; // rhs.m_halfExtents
    glm::dvec3 m_TA; // transpose(lhs.m_axes) * (rhs.m_center - lhs.m_center)
    glm::dvec3 m_TB; // transpose(rhs.m_axes) * (rhs.m_center - lhs.m_center)
};

/**
 * The implementation is based on the following paper:
 * https://www.geometrictools.com/Documentation/IntersectionLineBox.pdf
 * and code:
 * https://www.geometrictools.com/GTE/Mathematics/IntrLine3AlignedBox3.h
 */
class OrientedBox3D::LineIntersector
{
public:
    LineIntersector(OrientedBox3D const& obb, glm::dvec3 const& lineP0, glm::dvec3 const& lineP1);

    std::vector<glm::dvec3> intersection() const;

private:
    // Test whether the current clipped segment intersects the current
    // test plane.  If the return value is 'true', the segment does
    // intersect the plane and is clipped; otherwise, the segment is
    // culled (no intersection with box).
    static bool clip(double const denom, double const numer, double& t0, double& t1)
    {
        if (denom > 0.0)
        {
            if (numer > denom * t1)
            {
                return false;
            }
            if (numer > denom * t0)
            {
                t0 = numer / denom;
            }
            return true;
        }
        else if (denom < 0.0)
        {
            if (numer > denom * t0)
            {
                return false;
            }
            if (numer > denom * t1)
            {
                t1 = numer / denom;
            }
            return true;
        }
        else
        {
            return numer <= 0.0;
        }
    }

    glm::dvec3 m_lineOrigin;
    glm::dvec3 m_lineDirection;

    /**
     * The line origin (P0 point) in the box space, as it would be AABB with the center in the
     * origin.
     */
    glm::dvec3 m_inBoxLineOrigin;

    /**
     * The line direction in the box space, as it would be AABB with the center in the origin.
     */
    glm::dvec3 m_inBoxLineDirection;

    glm::dvec3 m_boxHalfExtents;
};

OrientedBox3D::OrientedBox3D(
    glm::dvec3 const& center, glm::dmat3 const& axes, glm::dvec3 const& halfExtents)
    : m_center(center)
    , m_axes(axes)
    , m_halfExtents(halfExtents)
{
}

OrientedBox3D::OrientedBox3D(AlignedBox3D const& alignedBox)
    : m_axes() // Builds the identity matrix.
    , m_halfExtents(0.5 * (alignedBox.maxCoords() - alignedBox.minCoords()))
{
    m_center = alignedBox.minCoords() + m_halfExtents;
}

bool
OrientedBox3D::containsPoint(glm::dvec3 const& point) const
{
    auto const localCoords = transpose(m_axes) * (point - m_center);
    return glm::clamp(localCoords, -m_halfExtents, m_halfExtents) == localCoords;
}

bool
OrientedBox3D::intersectsWith(OrientedBox3D const& other) const
{
    return Intersector(*this, other).haveIntersection();
}

AlignedBox3D
OrientedBox3D::alignedBoundingBox() const
{
    auto const halfExtents = glm::abs(m_axes * m_halfExtents);
    return AlignedBox3D(m_center - halfExtents, m_center + halfExtents);
}

std::vector<glm::dvec3>
OrientedBox3D::intersectionWithLine(glm::dvec3 const& p0,
    glm::dvec3 const& p1) const
{
    return LineIntersector(*this, p0, p1).intersection();
}


/*==================================== Intersector =================================*/

OrientedBox3D::Intersector::Intersector(OrientedBox3D const& lhs, OrientedBox3D const& rhs)
    : m_R(transpose(rhs.m_axes) * lhs.m_axes)
    , m_heA(lhs.m_halfExtents)
    , m_heB(rhs.m_halfExtents)
    , m_TA(transpose(lhs.m_axes) * (rhs.m_center - lhs.m_center))
    , m_TB(transpose(rhs.m_axes) * (rhs.m_center - lhs.m_center))
{
    m_absR[0] = glm::abs(m_R[0]);
    m_absR[1] = glm::abs(m_R[1]);
    m_absR[2] = glm::abs(m_R[2]);
}

bool
OrientedBox3D::Intersector::haveIntersection() const
{
    for (int i = 0; i < 3; ++i)
    {
        if (separableOnAiAxis(i) || separableOnBjAxis(i))
        {
            return false;
        }
    }

    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            if (separableOnAiCrossBjAxis(i, j))
            {
                return false;
            }
        }
    }

    return true;
}

bool
OrientedBox3D::Intersector::separableOnAiAxis(int i) const
{
    auto const projectedDistanceBetweenCenters = std::abs(m_TA[i]);

    auto const projectedRadiusA = m_heA[i];

    auto const projectedRadiusB =
        m_heB[0] * m_absR[i][0] +
        m_heB[1] * m_absR[i][1] +
        m_heB[2] * m_absR[i][2];

    return projectedDistanceBetweenCenters > projectedRadiusA + projectedRadiusB;
}

bool
OrientedBox3D::Intersector::separableOnBjAxis(int j) const
{
    auto const projectedDistanceBetweenCenters = std::abs(m_TB[j]);

    auto const projectedRadiusA =
        m_heA[0] * m_absR[0][j] +
        m_heA[1] * m_absR[1][j] +
        m_heA[2] * m_absR[2][j];

    auto const projectedRadiusB = m_heB[j];

    return projectedDistanceBetweenCenters > projectedRadiusA + projectedRadiusB;
}

bool
OrientedBox3D::Intersector::separableOnAiCrossBjAxis(int i, int j) const
{
    // Note that in this case we are not projecting on a unit vector, but on
    // cross(lhs.axes()[i], rhs.axes()[j]), which has a non-unit length.

    auto const projectedDistanceBetweenCenters = std::abs(
        m_TA[prev(i)] * m_R[next(i)][j] -
        m_TA[next(i)] * m_R[prev(i)][j]);

    auto const projectedRadiusA =
        m_heA[next(i)] * m_absR[prev(i)][j] +
        m_heA[prev(i)] * m_absR[next(i)][j];

    auto const projectedRadiusB =
        m_heB[next(j)] * m_absR[i][prev(j)] +
        m_heB[prev(j)] * m_absR[i][next(j)];

    return projectedDistanceBetweenCenters > projectedRadiusA + projectedRadiusB;
}

/*==================================== LineIntersector =================================*/

OrientedBox3D::LineIntersector::LineIntersector(OrientedBox3D const& obb, glm::dvec3 const& lineP0,
    glm::dvec3 const& lineP1)
    : m_lineOrigin(lineP0)
    , m_lineDirection(lineP1 - lineP0)
    , m_inBoxLineOrigin(transpose(obb.axes()) * (lineP0 - obb.center()))
    , m_inBoxLineDirection(obb.axes() * m_lineDirection)
    , m_boxHalfExtents(obb.halfExtents())
{
}

std::vector<glm::dvec3>
OrientedBox3D::LineIntersector::intersection() const
{
    std::vector<glm::dvec3> result;

    // Find intersection points as parameters for the line: t0, t1.
    // 
    // The line t-values are in the interval (-infinity,+infinity).
    // Clip the line against all six planes of an aligned box in
    // centered form. The result.numIntersections is
    //   0, no intersection
    //   1, intersect in a single point (t0 is line parameter of point)
    //   2, intersect in a segment (line parameter interval is [t0,t1])
    double t0 = -std::numeric_limits<double>::max();
    double t1 = std::numeric_limits<double>::max();
    if (clip(+m_inBoxLineDirection[0], -m_inBoxLineOrigin[0] - m_boxHalfExtents[0], t0, t1) &&
        clip(-m_inBoxLineDirection[0], +m_inBoxLineOrigin[0] - m_boxHalfExtents[0], t0, t1) &&
        clip(+m_inBoxLineDirection[1], -m_inBoxLineOrigin[1] - m_boxHalfExtents[1], t0, t1) &&
        clip(-m_inBoxLineDirection[1], +m_inBoxLineOrigin[1] - m_boxHalfExtents[1], t0, t1) &&
        clip(+m_inBoxLineDirection[2], -m_inBoxLineOrigin[2] - m_boxHalfExtents[2], t0, t1) &&
        clip(-m_inBoxLineDirection[2], +m_inBoxLineOrigin[2] - m_boxHalfExtents[2], t0, t1))
    {
        // Calculate points to the world space:
        if (t1 > t0)
        {
            result.push_back(m_lineOrigin + t0 * m_lineDirection);
            result.push_back(m_lineOrigin + t1 * m_lineDirection);
        }
        else
        {
            result.push_back(m_lineOrigin + t0 * m_lineDirection);
        }
    }

    return result;
}
