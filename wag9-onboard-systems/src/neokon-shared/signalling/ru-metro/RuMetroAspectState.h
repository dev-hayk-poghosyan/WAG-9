#pragma once

#include "neokon-shared/PlusEnum.h"
#include "neokon-shared/signalling/LegacySignalLightState.h"

#include <optional>

/**
 * All valid combinations of Russian Metro signals.
 *
 * @note The numeric values are saved into the scenario.
 */
enum class RuMetroAspectState
{
    Undefined = 0,  //Undefined state (for Stop signals)

    //Auto 
    Red = 1,        //The next section is closed.
    RedYellow = 2,  //The next section is clear.
    Yellow = 3,     //The next two sections are clear.
    YellowGreen = 4, //The next three sections are clear.
    Green = 5,       //For GR - the next 2 - 4 sections are clear,
                     //for YGR - the next 3 - 4 sections are clear,
                     //for YYGR - the next 4 sections are clear.
    GreenMore = 6,   //The next 5 or more crear sections (for Als code Green80).

    //Enter and exit semiauto
    DoubleYellow = 7,//The next section is clear and diverging. The section after that is closed.
    DoubleYellowTopFlashing = 8, //The next section is clear and diverging. The section after that is clear.

    //Shunting
    White = 9, //The next section is clear.
    WhiteWhite = 10, //The next section is clear, direction from Depo to Portal.
    FlashingYellow = 11, //The next section is clear, direction from Portal to Depo.

    //Invitation Aspect
    InvitationWhite = 12, //The next section state is undetermined.

    Blue = 13, //Train can move according to control frequency indication.

    Off = 14 //All lenses are off, train can move according to control frequency indication.
};

/**
 * This function is a fall-back in case if legacy signal state is used incorrectly when Ru-metro
 * signal state is retrieved from legacy signal.
 */
RuMetroAspectState ruMetroAspectStateFromLegacySignalLightState(LegacySignalLightState state);
