#pragma once

#include "DirectionMark.h"

/**
 * Represents a switcher mark for non-electrified zone.
 * @see documentation to SupplyChangeMark.
 */
class SwitcherMark : public DirectionMark
{
public:
    SwitcherMark(const std::string& name, const glm::dmat4& nodeTransform,
        bool isActive);

    virtual SwitcherMark* switcherMark() override { return this; }
    virtual DirectionMark* directionMark() override { return nullptr; }
    virtual SupplyChangeWarningMark* supplyChangeWarningMark() override { return nullptr; }

    void setActive(bool isActive);
    bool isActive() const { return m_isActive; }
private:
    bool m_isActive{ true };
};
