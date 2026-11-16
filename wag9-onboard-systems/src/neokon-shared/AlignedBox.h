#pragma once

#include <glm/glm.hpp>

/**
 * Represents the box with axes aligned with the world's axes, depending on used Vec
 * types, which could be any of the glm vector types:
 * glm::dvec2, glm::dvec3, glm::vec2 and etc.
 */
template<class Vec>
class AlignedBox
{
public:
    AlignedBox() {}
    /**
     * @param minCoords The vector of minimum (x, y, z) coordinates occupied by the box.
     * @param maxCoords The vector of maximum (x, y, z) coordinates occupied by the box.
     */
    AlignedBox(Vec const& minCoords, Vec const& maxCoords)
        : m_minCoords(minCoords)
        , m_maxCoords(maxCoords)
    {
    }

    /**
     * The vector of minimum (x, y, z) coordinates occupied by the box.
     */
    Vec const& minCoords() const { return m_minCoords; }

    /**
     * The vector of maximu (x, y, z) coordinates occupied by the box.
     */
    Vec const& maxCoords() const { return m_maxCoords; }

    /**
     * Returns true if the given point lies inside the box. The box boundaries are considered
     * to be a part of the box.
     */
    bool containsPoint(Vec const& point) const
    {
        return glm::clamp(point, m_minCoords, m_maxCoords) == point;
    }

    /**
     * Returns true if there is at least one point that's contained in both boxes.
     */
    bool intersectsWith(AlignedBox<Vec> const& other) const
    {
        auto const overlaps =
            glm::min(m_maxCoords, other.m_maxCoords) -
            glm::max(m_minCoords, other.m_minCoords);

        // If no element of overlaps is negative, we return true.
        return glm::min(overlaps, Vec(0.0)) == Vec(0.0);
    }

    /**
     * Extends this box to include another one.
     */
    AlignedBox<Vec>& operator|=(AlignedBox<Vec> const& other)
    {
        m_minCoords = glm::min(m_minCoords, other.minCoords());
        m_maxCoords = glm::max(m_maxCoords, other.maxCoords());
        return *this;
    }

    /**
     * Returns box expanded on both min and max sides by @p extents.
     */
    AlignedBox<Vec> expandedBy(Vec const& extents) const
    {
        return AlignedBox<Vec>(m_minCoords - extents, m_maxCoords + extents);
    }

private:
    Vec m_minCoords;
    Vec m_maxCoords;
};

template<typename Vec>
AlignedBox<Vec> operator|(AlignedBox<Vec> const& lhs, AlignedBox<Vec> const& rhs)
{
    return AlignedBox<Vec>(
        glm::min(lhs.minCoords(), rhs.minCoords()),
        glm::max(lhs.maxCoords(), rhs.maxCoords()));
}

using AlignedBox2D = AlignedBox<glm::dvec2>;
using AlignedBox3D = AlignedBox<glm::dvec3>;
