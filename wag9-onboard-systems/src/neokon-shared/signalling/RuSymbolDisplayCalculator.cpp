#pragma once

#include "RuSymbolDisplayCalculator.h"

/**
 * Materials description see at:
 * http://git.neokon.local:22580/root/GlobalTerrainEditor-release/wikis/signalling-symbolic-labels
 */
RuSymbolDisplayState
RuSymbolDisplayCalculator::getDisplayState(const std::string& mnemonics)
{
    const auto& map = getStringToSymbolDisplayMap();
    auto dataIter = map.find(mnemonics);
    return dataIter == map.end() ?
        RuSymbolDisplayState::Error :
        (RuSymbolDisplayState)(*dataIter).second;
}

std::string
RuSymbolDisplayCalculator::getDisplayValue(RuSymbolDisplayState state)
{
    const auto& map = getSymbolDisplayToStringMap();
    auto dataIter = map.find((int)state);
    return dataIter == map.end() ? "?ERROR" : (*dataIter).second;
}

std::string const&
RuSymbolDisplayCalculator::getMaterialName(RuSymbolDisplayState state)
{
    if (m_symbolDisplayMaterials.empty())
        ensureDisplayMaterials();

    return m_symbolDisplayMaterials[state];
}

void
RuSymbolDisplayCalculator::ensureDisplayMaterials()
{
    int i = 0;
    m_symbolDisplayMaterials[(RuSymbolDisplayState)i] = "OFF";
    while (i++ < 10) // 0 - 9
    {
        m_symbolDisplayMaterials[(RuSymbolDisplayState)i] = std::to_string(i - 1);
    }

    m_symbolDisplayMaterials[(RuSymbolDisplayState)i++] = "A";        // А
    m_symbolDisplayMaterials[(RuSymbolDisplayState)i++] = "rus-Be";   // Б
    m_symbolDisplayMaterials[(RuSymbolDisplayState)i++] = "B";        // В
    m_symbolDisplayMaterials[(RuSymbolDisplayState)i++] = "rus-Ge";   // Г
    m_symbolDisplayMaterials[(RuSymbolDisplayState)i++] = "rus-De";   // Д
    m_symbolDisplayMaterials[(RuSymbolDisplayState)i++] = "E";        // Е
    m_symbolDisplayMaterials[(RuSymbolDisplayState)i++] = "rus-Je";   // Ж 
    m_symbolDisplayMaterials[(RuSymbolDisplayState)i++] = "rus-Z";    // З
    m_symbolDisplayMaterials[(RuSymbolDisplayState)i++] = "rus-I";    // И
    m_symbolDisplayMaterials[(RuSymbolDisplayState)i++] = "K";        // К
    m_symbolDisplayMaterials[(RuSymbolDisplayState)i++] = "rus-L";    // Л
    m_symbolDisplayMaterials[(RuSymbolDisplayState)i++] = "M";        // М
    m_symbolDisplayMaterials[(RuSymbolDisplayState)i++] = "H";        // Н
    m_symbolDisplayMaterials[(RuSymbolDisplayState)i++] = "O";        // О 
    m_symbolDisplayMaterials[(RuSymbolDisplayState)i++] = "rus-P";    // П
    m_symbolDisplayMaterials[(RuSymbolDisplayState)i++] = "P";        // Р
    m_symbolDisplayMaterials[(RuSymbolDisplayState)i++] = "C";        // С
    m_symbolDisplayMaterials[(RuSymbolDisplayState)i++] = "T";        // Т
    m_symbolDisplayMaterials[(RuSymbolDisplayState)i++] = "rus-U";    // У
    m_symbolDisplayMaterials[(RuSymbolDisplayState)i++] = "rus-F";    // Ф 
    m_symbolDisplayMaterials[(RuSymbolDisplayState)i++] = "X";        // Х 
    m_symbolDisplayMaterials[(RuSymbolDisplayState)i++] = "rus-Ce";   // Ц
    m_symbolDisplayMaterials[(RuSymbolDisplayState)i++] = "rus-Che";  // Ч
    m_symbolDisplayMaterials[(RuSymbolDisplayState)i++] = "rus-Sha";  // Ш
    m_symbolDisplayMaterials[(RuSymbolDisplayState)i++] = "rus-Scha"; // Щ
    m_symbolDisplayMaterials[(RuSymbolDisplayState)i++] = "rus-E";    // Э
    m_symbolDisplayMaterials[(RuSymbolDisplayState)i++] = "rus-Yu";   // Ю
    m_symbolDisplayMaterials[(RuSymbolDisplayState)i++] = "rus-Ya";   // Я

    m_symbolDisplayMaterials[(RuSymbolDisplayState)i++] = "Direction-Forward";   // |
    m_symbolDisplayMaterials[(RuSymbolDisplayState)i++] = "Direction-Horizontal";// -
    m_symbolDisplayMaterials[(RuSymbolDisplayState)i++] = "Direction-toRight";    // /
    m_symbolDisplayMaterials[(RuSymbolDisplayState)i++] = "Direction-toLeft";   // \

    m_symbolDisplayMaterials[(RuSymbolDisplayState)i++] = "Arrow-Up";    // ↑
    m_symbolDisplayMaterials[(RuSymbolDisplayState)i++] = "Arrow-Down";  // ↓
    m_symbolDisplayMaterials[(RuSymbolDisplayState)i++] = "Arrow-Left";  // ←
    m_symbolDisplayMaterials[(RuSymbolDisplayState)i++] = "Arrow-Right"; // →
}

std::unordered_map<int, std::string>&
RuSymbolDisplayCalculator::getSymbolDisplayToStringMap()
{
    if (m_symbolDisplayToString.empty())
    {
        int i = 0;
        m_symbolDisplayToString[i++] = "_"; //Turned off
        while (i++ < 10) // 0 - 9
        {
            m_symbolDisplayToString[i] = std::to_string(i - 1);
        }

        m_symbolDisplayToString[i++] = "А";
        m_symbolDisplayToString[i++] = "Б";
        m_symbolDisplayToString[i++] = "В";
        m_symbolDisplayToString[i++] = "Г";
        m_symbolDisplayToString[i++] = "Д";
        m_symbolDisplayToString[i++] = "Е";
        m_symbolDisplayToString[i++] = "Ж";
        m_symbolDisplayToString[i++] = "З";
        m_symbolDisplayToString[i++] = "И";
        m_symbolDisplayToString[i++] = "К";
        m_symbolDisplayToString[i++] = "Л";
        m_symbolDisplayToString[i++] = "М";
        m_symbolDisplayToString[i++] = "Н";
        m_symbolDisplayToString[i++] = "О";
        m_symbolDisplayToString[i++] = "П";
        m_symbolDisplayToString[i++] = "Р";
        m_symbolDisplayToString[i++] = "С";
        m_symbolDisplayToString[i++] = "Т";
        m_symbolDisplayToString[i++] = "У";
        m_symbolDisplayToString[i++] = "Ф";
        m_symbolDisplayToString[i++] = "Х";
        m_symbolDisplayToString[i++] = "Ц";
        m_symbolDisplayToString[i++] = "Ч";
        m_symbolDisplayToString[i++] = "Ш";
        m_symbolDisplayToString[i++] = "Щ";
        m_symbolDisplayToString[i++] = "Э";
        m_symbolDisplayToString[i++] = "Ю";
        m_symbolDisplayToString[i++] = "Я";

        m_symbolDisplayToString[i++] = "|";
        m_symbolDisplayToString[i++] = "-";
        m_symbolDisplayToString[i++] = "/";
        m_symbolDisplayToString[i++] = "\\";

        m_symbolDisplayToString[i++] = "⇧";
        m_symbolDisplayToString[i++] = "⇩";
        m_symbolDisplayToString[i++] = "⇦";
        m_symbolDisplayToString[i++] = "⇨";
    }

    return m_symbolDisplayToString;
}

std::unordered_map<std::string, int>&
RuSymbolDisplayCalculator::getStringToSymbolDisplayMap()
{
    if (m_stringToSymbolDisplay.empty())
    {
        int i = 0;
        m_stringToSymbolDisplay["_"] = i;
        while (i++ < 10)
        {
            m_stringToSymbolDisplay[std::to_string(i - 1)] = i;
        }
        m_stringToSymbolDisplay["rA"] = i++; //А
        m_stringToSymbolDisplay["rB"] = i++; //Б 
        m_stringToSymbolDisplay["rV"] = i++; //В
        m_stringToSymbolDisplay["rG"] = i++; //Г
        m_stringToSymbolDisplay["rD"] = i++; //Д
        m_stringToSymbolDisplay["rYE"] = i++;//Е
        m_stringToSymbolDisplay["rZh"] = i++;//Ж
        m_stringToSymbolDisplay["rZ"] = i++; //З
        m_stringToSymbolDisplay["rI"] = i++; //И
        m_stringToSymbolDisplay["rK"] = i++; //К
        m_stringToSymbolDisplay["rL"] = i++; //Л
        m_stringToSymbolDisplay["rM"] = i++; //М
        m_stringToSymbolDisplay["rN"] = i++; //Н
        m_stringToSymbolDisplay["rO"] = i++; //О
        m_stringToSymbolDisplay["rP"] = i++; //П
        m_stringToSymbolDisplay["rR"] = i++; //Р
        m_stringToSymbolDisplay["rS"] = i++; //С
        m_stringToSymbolDisplay["rT"] = i++; //Т
        m_stringToSymbolDisplay["rU"] = i++; //У
        m_stringToSymbolDisplay["rF"] = i++; //Ф
        m_stringToSymbolDisplay["rH"] = i++; //Х
        m_stringToSymbolDisplay["rC"] = i++; //Ц
        m_stringToSymbolDisplay["rCh"] = i++;//Ч
        m_stringToSymbolDisplay["rSh"] = i++;//Ш
        m_stringToSymbolDisplay["rSha"] = i++;//Щ
        m_stringToSymbolDisplay["rE"] = i++; //Э
        m_stringToSymbolDisplay["rYU"] = i++;//Ю
        m_stringToSymbolDisplay["rYA"] = i++;//Я

        m_stringToSymbolDisplay["lV"] = i++; // |
        m_stringToSymbolDisplay["lH"] = i++; // -
        m_stringToSymbolDisplay["lS"] = i++; // /
        m_stringToSymbolDisplay["lBS"] = i++;// \ 

        m_stringToSymbolDisplay["arUp"]    = i++; // ↑
        m_stringToSymbolDisplay["arDown"]  = i++; // ↓
        m_stringToSymbolDisplay["arLeft"]  = i++; // ←
        m_stringToSymbolDisplay["arRight"] = i++; // → 
    }

    return m_stringToSymbolDisplay;
}

std::unordered_map<int, std::string>
RuSymbolDisplayCalculator::m_symbolDisplayToString;

std::unordered_map<std::string, int>
RuSymbolDisplayCalculator::m_stringToSymbolDisplay;

std::unordered_map<RuSymbolDisplayState, std::string>
RuSymbolDisplayCalculator::m_symbolDisplayMaterials;
