#pragma once

#include "neokon-shared/PlusEnum.h"

#include <optional>

/**
 * All valid combinations of Red, Green, one or two Yellow lights, and possibly green bars
 * on Russian-style exit signals.
 *
 * The shunting white light is not part of this state.
 *
 * It's very similar to RuEntranceMainAspectState, except the following differences:
 * 1. This one doesn't have a FlashingYellow state, which RuEntranceMainAspectState has.
 * 2. This one has DoubleYellowPlusGreenBar and DoubleYellowPlusTwoGreenBars, which
 *    RuEntranceMainAspectState doesn't have.
 * 3. Exit signals can have 1 or 2 yellow lights, so the set of valid states
 *    and their interpetations vary between these two variants.
 *
 * @note The numeric values are saved into the scenario.
 */
enum class RuExitMainAspectState
{
    /**
     * The next two sections are both clear.
     *
     * For 1-yellow signals, they may or may not be diverging.
     * For 2-yellow signals, they have to be straight, or else DoubleYellowTopFlashing
     * would have been used instead.
     */
    Green = 0,

    /**
     * The next section is clear. The section after that is closed.
     *
     * For 1-yellow signals, the section after the signal light is allowed to be diverging.
     * For 2-yellow signals, the section after the signal light has to be straight, or else
     * DoubleYellow would have been used instead.
     */
    Yellow = 1,

    /**
     * The next section is clear and diverging. The section after that is closed.
     *
     * For 1-yellow signals, this option is not valid. This kind of a road situation
     * should translate to Yellow for 1-yellow signals.
     */
    DoubleYellow = 2,

    /**
     * Same as DoubleYellow, but used when the signal light knows the switch ahead
     * is of type 1/18, which has low curvature, but now as low as 1/22.
     * In such a case, the maximum allowed speed when passing this signal is 60 km/h.
     * Exit signals with one green line are only placed in front of a 1/18 switch.
     */
    DoubleYellowPlusGreenBar = 5,

    /**
     * Same as DoubleYellow, but used when the signal light knows the
     * switch ahead is of type 1/22, which has low curvature.
     * In such a case, the maximum allowed speed when passing this signal is 60 km/h.
     * Exit signals with two green lines are only placed in front of a 1/22 switch.
     */
    DoubleYellowPlusTwoGreenBars = 6,

    /**
     * The next section is clear and diverging. The section after that is clear.
     * It may or may not be diverging.
     *
     * For 1-yellow signals, this option is not valid. This kind of a road situation
     * should translate to Green for 1-yellow signals.
     */
    DoubleYellowTopFlashing = 3,

    /**
     * Same as DoubleYellowTopFlashing, but used when the signal light knows the
     * switch ahead is of type 1/18, which has low curvature, but not as low as 1/22.
     * In such a case, the maximum allowed speed when passing this signal is 80 km/h.
     * Exit signals with two green lines are only placed in front of a 1/22 switch.
     */
     GreenFlashingPlusBottomYellowPlusGreenBar = 7,

    /**
     * Same as DoubleYellowTopFlashing, but used when the signal light knows the
     * switch ahead is of type 1/22, which has low curvature. In such a case,
     * the maximum allowed speed when passing this signal is 120 km/h.
     * Exit signals with two green lines are only placed in front of a 1/22 switch.
     */
    GreenFlashingPlusBottomYellowPlusTwoGreenBars = 8,

    /**
     * The next section is closed.
     */
    Red = 4
};

std::optional<RuExitMainAspectState> ruExitMainAspectStateFromInt(int state);

/**
 * Returns either DoubleYellow or DoubleYellowPlusGreenBar or DoubleYellowPlusTwoGreenBars
 * depending on the value of @p numGreenBars.
 */
RuExitMainAspectState ruExitMainAspectStateDoubleYellowOrReplacement(int numGreenBars);

/**
 * Returns either DoubleYellowTopFlashing or GreenFlashingPlusBottomYellowPlusGreenBar or
 * GreenFlashingPlusBottomYellowPlusTwoGreenBars depending on the  value of @p numGreenBars.
 */
RuExitMainAspectState ruExitMainAspectStateDoubleYellowTopFlashingOrReplacement(int numGreenBars);

int toProto(RuExitMainAspectState state);

int toProto(std::optional<RuExitMainAspectState> const& state);

int toProto(std::optional<std::optional<RuExitMainAspectState>> const& state);

void fromProto(int protoVal, std::optional<RuExitMainAspectState>& target);

void fromProto(int protoVal, std::optional<std::optional<RuExitMainAspectState>>& target);
