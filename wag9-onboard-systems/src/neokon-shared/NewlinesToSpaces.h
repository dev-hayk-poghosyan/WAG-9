#pragma once

#include <string>

/**
 * Replaces all '\n' characters with spaces and removes all '\r' characters.
 */
std::string newlinesToSpaces(std::string_view str);
