#pragma once

#include "neokon-shared/PlusEnum.h"

#include <optional>

/**
 * 3 and 4-aspect signals can be controlled, automatic or semi-automatic.
 *
 * @li An automatic signal can't be operated manually.
 * @li A semi-automatic signal can be in either automatic or manual mode.
 *     In manual mode it can be put into any valid state.
 * @li A controlled signal is just like a semi-automatic one, but the only
 *     state it can be put into manually, is Red.
 */
enum class UkMainAspectControlType
{
    Controlled = 0,
    Automatic = 1,
    SemiAutomatic = 2,
};

std::optional<UkMainAspectControlType> ukMainAspectControlTypeFromInt(int type);
