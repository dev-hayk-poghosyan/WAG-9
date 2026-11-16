#include "UkBannerRepeaterState.h"

std::wstring materialSymbolFor(UkBannerRepeaterState const state)
{
    switch (state)
    {
    case UkBannerRepeaterState::On:
        return L"banner-on";
    case UkBannerRepeaterState::Off:
        return L"banner-off";
    case UkBannerRepeaterState::Green:
        return L"banner-green";
    }

    return std::wstring();
}

std::wstring materialBillboardSymbolFor(UkBannerRepeaterState state)
{
    switch (state)
    {
    case UkBannerRepeaterState::On:
        return L"banner-on_billboard";
    case UkBannerRepeaterState::Off:
        return L"banner-off_billboard";
    case UkBannerRepeaterState::Green:
        return L"banner-green_billboard";
    }

    return std::wstring();
}

std::optional<UkBannerRepeaterState> ukBannerRepeaterStateFromInt(int const state)
{
    switch (state)
    {
    case +UkBannerRepeaterState::On:
    case +UkBannerRepeaterState::Off:
    case +UkBannerRepeaterState::Green:
        return static_cast<UkBannerRepeaterState>(state);
    }

    return std::nullopt;
}

int toProto(UkBannerRepeaterState state)
{
    return +state;
}

int toProto(std::optional<UkBannerRepeaterState> const& state)
{
    return state ? 1 + toProto(*state) : 0;
}

int toProto(std::optional<std::optional<UkBannerRepeaterState>> const& state)
{
    return state ? 1 + toProto(*state) : 0;
}

void fromProto(int protoVal, std::optional<UkBannerRepeaterState>& target)
{
    if (protoVal == 0)
    {
        target = std::nullopt;
    }
    else
    {
        target = ukBannerRepeaterStateFromInt(protoVal - 1);
    }
}

void fromProto(int protoVal, std::optional<std::optional<UkBannerRepeaterState>>& target)
{
    if (protoVal == 0)
    {
        target = std::nullopt;
    }
    else
    {
        target.emplace();
        fromProto(protoVal - 1, *target);
    }
}

