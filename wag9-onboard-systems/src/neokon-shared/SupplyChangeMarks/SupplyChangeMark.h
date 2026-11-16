#pragma once

#include <glm/glm.hpp>

#include <string>

class SwitcherMark;
class DirectionMark;
class SupplyChangeWarningMark;

/**
 * Represents supply change marks (diesel/contact wire).
 * A pair of SupplyChangeMark (SwitcherMark, DirectionMark) specifies the non-electrified zone.
 * The zone extends from the SwitcherMark to the branches on which
 * the DirectionMark is set.
 * Supply change warning marks are set ahead and behind the SwitcherMark on distance 380m. They
 * used to send warning signal to the train about the approaching to switcher.
 */
class SupplyChangeMark
{
public:
    virtual glm::dvec3 worldPosition() const = 0;
    virtual glm::dvec2 worldDirection() const = 0;
    virtual const std::string& name() const = 0;
    virtual const glm::dmat4& nodeTransform() const = 0;

    /**
     * This method is used in RailroadControl to generalize the handling of the marks.
     * The implementation must retain its ownership over the object returned.
     */
    virtual SwitcherMark* switcherMark() = 0;

    /**
     * This method is used in RailroadControl to generalize the handling of the marks.
     * The implementation must retain its ownership over the object returned.
     */
    virtual DirectionMark* directionMark() = 0;

    /**
     * This method is used in RailroadControl to generalize the handling of the marks.
     * The implementation must retain its ownership over the object returned.
     */
    virtual SupplyChangeWarningMark* supplyChangeWarningMark() = 0;
};
