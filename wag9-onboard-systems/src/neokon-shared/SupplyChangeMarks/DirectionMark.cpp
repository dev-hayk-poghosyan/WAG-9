#include "DirectionMark.h"

DirectionMark::DirectionMark(const std::string& name, const glm::dmat4& nodeTransform)
    : m_nodeTransform(nodeTransform)
    , m_name(name)
{}

glm::dvec3 DirectionMark::worldPosition() const
{
    return m_nodeTransform[3];
}

glm::dvec2 DirectionMark::worldDirection() const
{
    // Take Y axis.
    return m_nodeTransform[1];
}
