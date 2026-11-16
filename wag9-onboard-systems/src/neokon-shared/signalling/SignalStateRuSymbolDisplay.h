#pragma once

#include "RuSymbolDisplayState.h"
#include "SignalState.h"

#include <optional>

class ContainsRuSymbolDisplayState
{
public:
    virtual std::optional<bool> kguDispayValue() const { return std::optional<bool>(); }
    virtual RuSymbolDisplayState symbolDispayValue(unsigned int index) const = 0;
};

class SignalStateRuSymbolDisplay
{
public:
    struct PartialState
    {
        std::optional<RuSymbolDisplayState> symbolDisplay1;
        std::optional<RuSymbolDisplayState> symbolDisplay2;
        std::optional<bool> symbolDisplayKgu;

        PartialState() = default;

        void fromProto(int protoVal, unsigned int index);

        int toProto(unsigned int index) const;

        void fromProtoKGU(int protoVal);

        int toProtoKGU() const;

        /**
         * Sets signal Display state.
         * Returns TRUE if value is differ from current
         */
        bool setSymbolDispayValue(unsigned int index,
            const std::optional<RuSymbolDisplayState>& value);
        /**
         * Sets signal KGU state.
         * Returns TRUE if value is differ from current
         */
        bool setSymbolDispayValueKGU(bool value);
    };

    /**
     * Takes the fields from the partial state that are set, and writes
     * them to the corresponding fields in this object.
     *
     * Returns true if any fields of this object changed their values.
     */
    bool mergePartialState(const SignalStateRuSymbolDisplay::PartialState& state);

    /**
     * Returns the partial state containing all the data from the full state.
     */
    PartialState toPartialState() const;

    /**
     * Returns Symbol Diplay value.
     */
    RuSymbolDisplayState symbolDispayValue(unsigned int index) const;

    /**
     * Returns KGU Diplay value.
     */
    std::optional<bool> kguDispayValue() const;

    /**
     * Value to display on Symbolic Display-1.
     * Initiated if Signal contains Symbolic Display(s).
     */
    std::optional<RuSymbolDisplayState> m_symbolDisplay1;

    /**
     * Value to display on Symbolic Display-2.
     * Initiated if Signal contains 2 Symbolic Displays.
     */
    std::optional<RuSymbolDisplayState> m_symbolDisplay2;

    /**
     * Flag for Symbolic Display KGU:
     * nullopt - undefined, TRUE - show, FALSE - hide.
     */
    std::optional<bool> m_symbolDisplayKgu;
};
