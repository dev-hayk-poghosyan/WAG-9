#pragma once

#include "AlignedBox.h"

#include <glm/glm.hpp>

#include <vector>


/**
 * A box rotated arbitrarily in space.
 */
class OrientedBox3D
{
public:
    /**
     * @param center The center of the box.
     * @param axes The basis of the box's local coordinate system. That is, axes[i] is the
     *        unit vector in world space that corresponds to the ith axis in the box's
     *        local coordinate system. This basis must be orthonormal!
     * @param halfExtents The halved width (x-dimension), height (y-dimension), and depth
     *        (z-dimension) of the box.
     */
    OrientedBox3D(glm::dvec3 const& center, glm::dmat3 const& axes, glm::dvec3 const& halfExtents);

    /**
     * Builds an oriented box from an aligned one.
     */
    OrientedBox3D(AlignedBox3D const& alignedBox);

    /**
     * The center of the box.
     */
    glm::dvec3 const& center() const { return m_center; }

    void setCenter(glm::dvec3 const& center) { m_center = center; }

    /**
     * The basis of the box's local coordinate system. That is, axes[i] is the
     * unit vector in world space that corresponds to the ith axis in the box's
     * local coordinate system.
     */
    glm::dmat3 const& axes() const { return m_axes; }

    void setAxes(glm::dmat3 const& axes) { m_axes = axes; }

    /**
     * The halved width (x-dimension), height (y-dimension), and depth (z-dimension) of the box.
     */
    glm::dvec3 const& halfExtents() const { return m_halfExtents; }

    void setHalfDims(glm::dvec3 const& halfExtents) { m_halfExtents = halfExtents; }

    /**
     * Returns true if the given point lies inside the box. The box boundaries are considered
     * to be a part of the box, but the computation is inexact anyway, so that doesn't matter
     * in practice.
     */
    bool containsPoint(glm::dvec3 const& point) const;

    /**
     * Returns true if there is at least one point that's contained in both boxes.
     */
    bool intersectsWith(OrientedBox3D const& other) const;

    /**
     * Returns the axis-aligned bounding box of this box.
     */
    AlignedBox3D alignedBoundingBox() const;

    /**
     * The intersection of box with line will give 0, 1 or 2 different points, so if there is an
     * intersection, the returned vector will contain the intersection positions.
     */
    std::vector<glm::dvec3> intersectionWithLine(glm::dvec3 const& p0, glm::dvec3 const& p1) const;

private:
    class Intersector;
    class LineIntersector;

    glm::dvec3 m_center;
    glm::dmat3 m_axes;
    glm::dvec3 m_halfExtents;
};
