#include "RuMetroAlsCode.h"

#include "neokon-shared/UnitConv.h"

RuMetroAlsCode ruMetroAlsCodeFromLegacySignalLightState(LegacySignalLightState state)
{
    switch (state)
    {
    case LegacySignalLightState::Off:
        return RuMetroAlsCode::NoFrequency0;
    case LegacySignalLightState::White:
    case LegacySignalLightState::WhiteFlash:
    case LegacySignalLightState::Err01:
        return RuMetroAlsCode::NoFrequency0;
    case LegacySignalLightState::Green:
    case LegacySignalLightState::GreenWhite:
    case LegacySignalLightState::YellowFlash: // Unintuitive, but correct.
        return RuMetroAlsCode::Green80; // Or Green70 or Green60?
    case LegacySignalLightState::Yellow:
    case LegacySignalLightState::YellowYellow:
    case LegacySignalLightState::YellowYellowFlash:
        return RuMetroAlsCode::Yellow40;
    case LegacySignalLightState::Red:
    case LegacySignalLightState::RedWhiteFlash:
        return RuMetroAlsCode::Red0;
    }

    return RuMetroAlsCode::NoFrequency0;
}

RuMetroAlsCode ruMetroAlsCodeFromInt(int kmh)
{
    if (kmh < 40)
        return RuMetroAlsCode::Red0;
    if (kmh < 60)
        return RuMetroAlsCode::Yellow40;
    if (kmh < 70)
        return RuMetroAlsCode::Green60;
    if (kmh < 75)
        return RuMetroAlsCode::Green70;
    if (kmh < 80)
        return RuMetroAlsCode::Green75;

    return RuMetroAlsCode::Green80;
}

std::string ruMetroAlsCodeToStr(RuMetroAlsCode alsCode)
{
    switch (alsCode)
    {
    case RuMetroAlsCode::NoFrequency0:
    case RuMetroAlsCode::AbsoluteStop0:
        return u8"ОЧ";
    case RuMetroAlsCode::Red0:
        return "0";
    case RuMetroAlsCode::Yellow40:
        return "40";
    case RuMetroAlsCode::Green60:
        return "60";
    case RuMetroAlsCode::Green70:
        return "70";
    case RuMetroAlsCode::Green75:
        return "75";
    case RuMetroAlsCode::Green80:
        return "80";
    default:
        return "--";
    }
}

std::optional<double>
ruMetroSpeedLimitMsFromAlsCode(RuMetroAlsCode alsCode)
{
    switch (alsCode)
    {
    case RuMetroAlsCode::NoFrequency0:
        return std::nullopt;
    case RuMetroAlsCode::AbsoluteStop0:
        return 0.0;
    case RuMetroAlsCode::Red0:
        return 0.0;
    case RuMetroAlsCode::Yellow40:
        return UnitConv::kmh2ms(40.0);
    case RuMetroAlsCode::Green60:
        return UnitConv::kmh2ms(60.0);
    case RuMetroAlsCode::Green70:
        return UnitConv::kmh2ms(70.0);
    case RuMetroAlsCode::Green75:
        return UnitConv::kmh2ms(75.0);
    case RuMetroAlsCode::Green80:
        return UnitConv::kmh2ms(80.0);
    default:
        return std::nullopt;
    }
}
