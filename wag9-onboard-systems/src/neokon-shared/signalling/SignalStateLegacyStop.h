#pragma once

#include "SignalStateLegacy.h"

/**
 * This class is used in Repeater to get difference
 * between Stop and Shunt signals state.
 * TODO: refactor to use the modern signalling system instead of Legacy.
 */
class SignalStateLegacyStop : public SignalStateLegacy
{
};
