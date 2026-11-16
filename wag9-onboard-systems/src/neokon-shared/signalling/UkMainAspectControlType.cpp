#include "UkMainAspectControlType.h"

std::optional<UkMainAspectControlType> ukMainAspectControlTypeFromInt(int type)
{
    switch (type)
    {
    case +UkMainAspectControlType::Controlled:
    case +UkMainAspectControlType::Automatic:
    case +UkMainAspectControlType::SemiAutomatic:
        return static_cast<UkMainAspectControlType>(type);
    }

    return std::nullopt;
}
