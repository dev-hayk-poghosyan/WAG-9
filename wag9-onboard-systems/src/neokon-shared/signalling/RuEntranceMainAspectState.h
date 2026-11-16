#pragma once

#include "neokon-shared/PlusEnum.h"

#include <vector>
#include <optional>

/**
 * All valid combinations of Red, Green, two Yellow lights, and possibly green bars
 * on Russian-style entrance signals.
 *
 * The invitational flashing white light is not part of this state.
 *
 * @note The numeric values are saved into the scenario.
 */
enum class RuEntranceMainAspectState
{
    /**
     * The next two sections are both clear and straight.
     */
    Green = 0,

    /**
     * The next section is clear and straight. The section after that is closed.
     */
    Yellow = 1,

    /**
     * The next section is clear and diverging. The section after that is closed.
     */
    DoubleYellow = 2,

    /**
     * The next section is clear and diverging. The section after that is closed.
     * In such a case, the maximum allowed speed when passing this signal is 60 km/h.
     * Diverging on the switch with low curvature (1/18 e.g.)
     */
    DoubleYellowPlusGreenBar = 8,

    /**
     * The next section is clear and diverging. The section after that is closed.
     * In such a case, the maximum allowed speed when passing this signal is 60 km/h.
     * Diverging on the switch with low curvature (1/22 e.g.)
     */
    DoubleYellowPlusTwoGreenBars = 10,

    /**
     * The next section is clear and straight. The section after that is clear and diverging.
     */
    FlashingYellow = 3,

    /**
     * The next section is clear and diverging. The section after that is clear.
     * It may or may not be diverging.
     */
    DoubleYellowTopFlashing = 4,

    /**
     * The next section is clear and diverging. The section after that is clear.
     * In such a case, the maximum allowed speed when passing this signal is 80 km/h.
     * Diverging on the switch with low curvature (1/18 e.g.)
     */
    DoubleYellowTopFlashingPlusGreenBar = 9,

    /**
     * The next section is clear and diverging. The section after that is clear.
     * In such a case, the maximum allowed speed when passing this signal is 80 km/h.
     * Diverging on the switch with low curvature (1/22 e.g.)
     */
    DoubleYellowTopFlashingPlusTwoGreenBars = 11,
    
    /**
     * Same as DoubleYellowTopFlashing, but used when the signal light knows the
     * switch ahead is of type 1/18, which has low curvature, but not as low as 1/22.
     * In such a case, the maximum allowed speed when passing this signal is 80 km/h.
     * Exit signals with two green lines are only placed in front of a 1/22 switch.
     */
    GreenFlashingPlusBottomYellowPlusGreenBar = 6,

     /**
      * Same as DoubleYellowTopFlashing, but used when the signal light knows the
      * switch ahead is of type 1/22, which has low curvature. In such a case,
      * the maximum allowed speed when passing this signal is 120 km/h.
      * Exit signals with two green lines are only placed in front of a 1/22 switch.
      */
    GreenFlashingPlusBottomYellowPlusTwoGreenBars = 7,

    /**
     * The next section is closed.
     */
    Red = 5
};

std::optional<RuEntranceMainAspectState> ruEntranceMainAspectStateFromInt(int state);

/**
 * Returns collections of states with green bars depending on the value of @p numGreenBars.
 */
std::vector<RuEntranceMainAspectState> ruEntranceMainAspectStatesWithGreenBars(int numGreenBars);

int toProto(RuEntranceMainAspectState state);

int toProto(std::optional<RuEntranceMainAspectState> const& state);

int toProto(std::optional<std::optional<RuEntranceMainAspectState>> const& state);

void fromProto(int protoVal, std::optional<RuEntranceMainAspectState>& target);

void fromProto(int protoVal, std::optional<std::optional<RuEntranceMainAspectState>>& target);
