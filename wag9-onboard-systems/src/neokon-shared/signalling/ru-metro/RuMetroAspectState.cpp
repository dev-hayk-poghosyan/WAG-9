#include "RuMetroAspectState.h"

RuMetroAspectState ruMetroAspectStateFromLegacySignalLightState(LegacySignalLightState state)
{
    switch (state)
    {
    case LegacySignalLightState::Off:
        return RuMetroAspectState::Undefined;
    case LegacySignalLightState::White:
    case LegacySignalLightState::WhiteFlash:
    case LegacySignalLightState::Err01:
        return RuMetroAspectState::White;
    case LegacySignalLightState::Green:
    case LegacySignalLightState::GreenWhite:
    case LegacySignalLightState::YellowFlash: // Unintuitive, but correct.
        return RuMetroAspectState::Green;
    case LegacySignalLightState::Yellow:
    case LegacySignalLightState::YellowYellow:
    case LegacySignalLightState::YellowYellowFlash:
        return RuMetroAspectState::Yellow;
    case LegacySignalLightState::Red:
    case LegacySignalLightState::RedWhiteFlash:
        return RuMetroAspectState::Red;
    }

    return RuMetroAspectState::Undefined;
}
