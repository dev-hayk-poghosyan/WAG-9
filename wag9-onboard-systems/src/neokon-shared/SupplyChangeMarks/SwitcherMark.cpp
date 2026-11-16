#include "SwitcherMark.h"

SwitcherMark::SwitcherMark(const std::string& name,
    const glm::dmat4& nodeTransform, bool isActive)
    : DirectionMark(name, nodeTransform)
    , m_isActive(isActive)
{
}

void SwitcherMark::setActive(bool isActive)
{
    m_isActive = isActive;
}
