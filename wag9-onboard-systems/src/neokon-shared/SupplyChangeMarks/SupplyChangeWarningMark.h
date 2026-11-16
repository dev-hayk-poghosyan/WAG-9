#pragma once

#include "DirectionMark.h"

/**
 * Represents a supply change warning mark for non-electrified zone.
 * @see documentation to SupplyChangeMark.
 */
class SupplyChangeWarningMark : public DirectionMark
{
public:
    SupplyChangeWarningMark(const std::string& name, const glm::dmat4& nodeTransform,
        const std::string& hostMarkName, bool isAheadMark);

    virtual SwitcherMark* switcherMark() override { return nullptr; }
    virtual DirectionMark* directionMark() override { return nullptr; }
    virtual SupplyChangeWarningMark* supplyChangeWarningMark() override { return this; }

    const std::string& hostMarkName() const { return m_hostMarkName; }
    bool isAheadMark() const { return m_isAheadMark; }

private:
    std::string m_hostMarkName;
    bool m_isAheadMark{ false };
};
