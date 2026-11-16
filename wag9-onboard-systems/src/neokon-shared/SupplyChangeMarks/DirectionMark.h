#pragma once

#include "SupplyChangeMark.h"

#include <glm/glm.hpp>

#include <string>

/**
 * Represents a direction mark for non-electrified zone.
 * @see documentation to SupplyChangeMark.
 */
class DirectionMark : public SupplyChangeMark
{
public:
    DirectionMark(const std::string& name, const glm::dmat4& nodeTransform);
    virtual glm::dvec3 worldPosition() const override;
    virtual glm::dvec2 worldDirection() const override;
    virtual const std::string& name() const override { return m_name; }
    virtual const glm::dmat4& nodeTransform() const override { return m_nodeTransform; }

    virtual SwitcherMark* switcherMark() override { return nullptr; }
    virtual DirectionMark* directionMark() override { return this; }
    virtual SupplyChangeWarningMark* supplyChangeWarningMark() override { return nullptr; }
private:
    glm::dmat4 m_nodeTransform;
    std::string m_name;
};
