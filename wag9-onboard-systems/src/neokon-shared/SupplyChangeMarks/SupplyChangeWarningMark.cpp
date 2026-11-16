#include "SupplyChangeWarningMark.h"

SupplyChangeWarningMark::SupplyChangeWarningMark(const std::string& name,
    const glm::dmat4& nodeTransform, const std::string& hostMarkName, bool isAheadMark)
    : DirectionMark(name, nodeTransform)
    , m_hostMarkName(hostMarkName)
    , m_isAheadMark(isAheadMark)
{
}
