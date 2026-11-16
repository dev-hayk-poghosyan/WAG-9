#pragma once

#include "BooleanAspectState.h"
#include "CommonCabinSignalColor.h"
#include "RuSymbolDisplayState.h"
#include "RuExitMainAspectState.h"
#include "SignalState.h"
#include "SignalStateAspectBot.h"
#include "SignalStateAspectRu.h"
#include "SignalStateRuSymbolDisplay.h"

#include <optional>

class SignalStateRuExit
    : public SignalState
    , public ContainsRuSymbolDisplayState
    , private SignalStateAspectBot
    , private SignalStateAspectRu
{
public:
    struct PartialState
    {
        /**
         * Why haven't we wrapped mainAspectState into an extra std::optional?
         * Because this field being optional in the outer class is something we
         * had to do rather than something we wanted to do. To minimise the extra
         * complexity that brings, we haven't wrapped it into an extra std::optional
         * here. After all, we don't want a partial state update being able to set
         * the main aspect state to null.
         *
         * All other fields we do wrap into an extra std::optional.
         *
         * @see SignalStateRuExit::m_mainAspectState
         */
        std::optional<RuExitMainAspectState> mainAspectState;

        std::optional<std::optional<RuExitMainAspectState>> manuallyRequestedMainAspectState;

        std::optional<BooleanAspectState> shuntingWhiteRequested;

        SignalStateRuSymbolDisplay::PartialState symbolDisplays;

        PartialState() = default;

        explicit PartialState(std::string const& serialized);

        std::string serialize() const;

        /**
         * Sets symbol display state.
         * @param index: 0 - first display, 1 - second display.
         * @param value: diplay symbol mnemonics.
         * Returns true if display state was changed.
         */
        bool setSymbolDisplayState(unsigned int index, RuSymbolDisplayState value);
    };

    static CommonCabinSignalColor commonCabinSignalColor(RuExitMainAspectState state);

    static std::vector<RuExitMainAspectState> parseAspectState(const std::string& state);

    /**
     * Returns the partial state containing all the data from the full state.
     */
    PartialState toPartialState() const;

    virtual std::unique_ptr<SignalState> clone() const override;

    //Override for ContainsRuSymbolDisplayState
    virtual RuSymbolDisplayState symbolDispayValue(unsigned int index) const override;

    virtual bool mergeSerializedPartialState(std::string const& serialized) override;

    /**
     * Takes the fields from the partial state that are set, and writes
     * them to the corresponding fields in this object.
     *
     * Returns true if any fields of this object changed their values.
     */
    bool mergePartialState(PartialState const& partialState);

    virtual SignalStateAspectBot const* botAspect() const override { return this; }

    virtual SignalStateAspectRu const* ruAspect() const override { return this; }

    std::optional<RuExitMainAspectState> mainAspectState() const { return m_mainAspectState; }

    std::optional<RuExitMainAspectState> manuallyRequestedMainAspectState() const {
        return m_manuallyRequestedMainAspectState;
    }

    /**
     * Returns mainAspectState() if it's set, or manuallyRequestedMainAspectState() otherwise.
     */
    std::optional<RuExitMainAspectState> actualOrRequestedMainAspectState() const;

    /**
     * Returns the main aspect state to be displayed. Returns the same value as
     * actualOrRequestedMainAspectState(), except when it's Red and shuntingWhiteRequested() is On,
     * std::nullopt is returned. In other words, shunting White suppresses main aspect Red.
     */
    std::optional<RuExitMainAspectState> visualMainAspectState() const;

    BooleanAspectState shuntingWhiteRequested() const { return m_shuntingWhiteRequested; }

    /**
     * Returns the shunting white state to be displayed. If actualOrRequestedMainAspectState()
     * is Red, returns whatever shuntingWhiteRequested() returns. Otherwise returns Off.
     * In other words, shunting white is only visible if main aspect is Red.
     *
     * @note The shunting white light doesn't flash.
     */
    BooleanAspectState visualShuntingWhiteState() const;

private:
    virtual CommonCabinSignalColor commonCabinSignalColor() const override;

    /**
     * Why did we make the actual signal's state optional? Because that way, we can display
     * the manual state if the actual state is not set. This allows us to see the signals
     * displaying the state we've choosen manually, until a connection to TrainSim is established
     * and it sets whatever the actual state it wants (taking our manual state request into account).
     */
    std::optional<RuExitMainAspectState> m_mainAspectState;

    std::optional<RuExitMainAspectState> m_manuallyRequestedMainAspectState;

    /**
     * This one may be set to On at any point in time, but it only lits up when m_mainAspectState
     * is Red. In that case, main aspect Red is not displayed. In other words, the shunting White
     * is suppressed by any main aspect state except Red. Red is itself suppressed by shunting
     * White.
     */
    BooleanAspectState m_shuntingWhiteRequested = BooleanAspectState::Off;

    SignalStateRuSymbolDisplay m_symbolDisplays;
};
