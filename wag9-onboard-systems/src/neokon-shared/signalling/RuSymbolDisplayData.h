#pragma once

#include "neokon-shared/signalling/CommonCabinSignalColor.h"
#include "neokon-shared/signalling/RuSymbolDisplayCalculator.h"
#include "neokon-shared/signalling/RuSymbolDisplayState.h"

#include <functional>
#include <string>
#include <vector>
#include <unordered_map>

#include <boost/algorithm/string.hpp>

template <typename SignalState, typename AspectState>
struct RuSymbolDisplayData
{
public:
    RuSymbolDisplayData() = default;

    /**
     * @Param data String with separated by '_': 
     *  - symbol to display mnemonics (see RuSymbolDisplayCalculator::getStringToSymbolDisplayMap),
     *  - diplay index: 1 or 2, optional, by default: 1,
     */
    RuSymbolDisplayData(const std::string& data)
    {
        std::vector<std::string> parts;
        boost::split(parts, data, boost::is_any_of("_"));

        if (!parts.empty())
        {
            auto len = parts.size();
            m_state = len ?
                RuSymbolDisplayCalculator::getDisplayState(parts[0]) :
                RuSymbolDisplayState::Error;

            m_displayIndex = (len > 1) ? std::stoi(parts[1]) - 1 : 0;
        }
        else
        {
            m_state = RuSymbolDisplayState::Error;
        }
    }

    /**
     * @Param data String with separated by '_':
     *  - symbol to display mnemonics (see RuSymbolDisplayCalculator::getStringToSymbolDisplayMap),
     *  - diplay index: 1 or 2, optional, by default: 1,
     *  - signal aspext to accept mnemonics (see RuMetroSymbolDisplayData::getCache),
     *       optional, by default - any no stop aspext.
     * i.e. 'rA_2_IW' - display cyrillic A in display 2 only for Invitation White aspect,
     *      '1' - display '1' in display 1 only for any no-stop aspect.
     */
    RuSymbolDisplayData(const std::wstring& data, const SignalState& signalState)
        : RuSymbolDisplayData(std::string(data.begin(), data.end()))
    {
        if (m_state != RuSymbolDisplayState::Error)
        {
            std::vector<std::string> parts;
            boost::split(parts, data, boost::is_any_of("_"));
            auto len = parts.size();
            if (len > 2)
            {
                std::vector<std::string> values;
                boost::split(values, parts[2], boost::is_any_of(","));
                for (const auto& val : values)
                {
                    const std::vector<AspectState>& aspectStates = signalState.parseAspectState(val);
                    if (!aspectStates.empty())
                    {
                        m_forSignalStates.insert(m_forSignalStates.end(), aspectStates.begin(), aspectStates.end());
                    }
                }
            }
        }
    }

    /**
     * Displayed state could be acceptable only for some Signal Aspect States.
     * This function checks if aspect state acceptable or no.
     * Returns true if acceptable.
     */
    bool acceptedAspectState(AspectState state, const SignalState& signalState) const
    {
        return m_forSignalStates.size() ?
            (std::find(m_forSignalStates.begin(), m_forSignalStates.end(), state) != m_forSignalStates.end()) :
            signalState.commonCabinSignalColor(state) != CommonCabinSignalColor::Red;
    }

    RuSymbolDisplayState getDisplayState() const
    {
        return m_state;
    }

    unsigned int getDisplayIndex() const
    {
        return m_displayIndex;
    }

private:

    std::vector<AspectState> m_forSignalStates;

    RuSymbolDisplayState m_state = RuSymbolDisplayState::Off;

    unsigned int m_displayIndex = 0;
};
