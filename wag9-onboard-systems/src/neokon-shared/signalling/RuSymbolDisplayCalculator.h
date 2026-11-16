#pragma once

#include "RuSymbolDisplayState.h"

#include <string>
#include <unordered_map>

/**
 * Performs work with RuSymbolDisplayState values.
 */
struct RuSymbolDisplayCalculator
{
public:

    /**
     * Returns Symbol Display string value.
     * @param state Symbolic Display state.
     */
    static std::string getDisplayValue(RuSymbolDisplayState state);

    /**
     * Returns Symbol Display state.
     * @param mnemonics String mnemonics of displayed symbol.
     */
    static RuSymbolDisplayState getDisplayState(const std::string& mnemonics);

    static const std::string& getMaterialName(RuSymbolDisplayState state);

private:
    static void ensureDisplayMaterials();
    static std::unordered_map<int, std::string>& getSymbolDisplayToStringMap();
    static std::unordered_map<std::string, int>& getStringToSymbolDisplayMap();

    static std::unordered_map<std::string, int> m_stringToSymbolDisplay;

    static std::unordered_map<RuSymbolDisplayState, std::string> m_symbolDisplayMaterials;

    static std::unordered_map<int, std::string> m_symbolDisplayToString;
};
