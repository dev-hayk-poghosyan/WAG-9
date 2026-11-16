#pragma once

#include "BooleanAspectState.h"
#include "CommonCabinSignalColor.h"
#include "RuEntrance2MainAspectState.h"
#include "RuEntrance2MainAspectVisualState.h"
#include "SignalState.h"
#include "SignalStateAspectBot.h"
#include "SignalStateAspectRu.h"
#include "SignalStateAspectRuEntrance.h"
#include "SignalStateRuSymbolDisplay.h"

#include "neokon-shared/signalling/LampMode.h"

#include <optional>

class SignalStateRuEntrance2
    : public ContainsRuSymbolDisplayState
    , public SignalState
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
         * @see SignalStateRuEntranceAdditional::m_mainAspectState
         */
        std::optional<RuEntrance2MainAspectState> mainAspectState;

        std::optional<std::optional<RuEntrance2MainAspectState>>
            manuallyRequestedMainAspectState;

        std::optional<BooleanAspectState> invitationalWhiteRequested;

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

    static CommonCabinSignalColor commonCabinSignalColor(RuEntrance2MainAspectState state);

    static std::vector<RuEntrance2MainAspectState> parseAspectState(const std::string& state);

    /**
     * Returns the partial state containing all the data from the full state.
     */
    PartialState toPartialState() const;

    virtual std::unique_ptr<SignalState> clone() const override;

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

    std::optional<RuEntrance2MainAspectState> mainAspectState() const {
        return m_mainAspectState;
    }

    std::optional<RuEntrance2MainAspectState> manuallyRequestedMainAspectState() const {
        return m_manuallyRequestedMainAspectState;
    }

    /**
     * Returns mainAspectState() if it's set, or manuallyRequestedMainAspectState() otherwise.
     */
    std::optional<RuEntrance2MainAspectState> actualOrRequestedMainAspectState() const;

    //override for ContainsRuSymbolDisplayState
    virtual RuSymbolDisplayState symbolDispayValue(unsigned int index) const override;

    /**
     * Returns the main aspect state to be displayed.
     *
     * This value is based on actualOrRequestedMainAspectState() but also takes into account
     * the fact shunting White suppresses main aspect Red.
     */
    RuEntrance2MainAspectVisualState visualMainAspectState() const;

    BooleanAspectState invitationalWhiteRequested() const { return m_invitationalWhiteRequested; }

    /**
     * The invitational white will be flashing when it's requested and
     * actualOrRequestedMainAspectState() returns Red.
     */
    LampMode visualInvitationalWhiteState() const;

    BooleanAspectState shuntingWhiteRequested() const { return m_shuntingWhiteRequested; }

    /**
     * The shunting white will be lit when it's requested while invitational white is
     * not, and actualOrRequestedMainAspectState() returns Red.
     */
    LampMode visualShuntingWhiteState() const;

private:
    virtual CommonCabinSignalColor commonCabinSignalColor() const override;

    /**
     * Why did we make the actual signal's state optional? Because that way, we can display
     * the manual state if the actual state is not set. This allows us to see the signals
     * displaying the state we've choosen manually, until a connection to TrainSim is established
     * and it sets whatever the actual state it wants (taking our manual state request into account).
     */
    std::optional<RuEntrance2MainAspectState> m_mainAspectState;

    std::optional<RuEntrance2MainAspectState> m_manuallyRequestedMainAspectState;

    /**
     * If set to On, the invitational White light will be flashing, but only if
     * m_mainAspectState is Red.
     */
    BooleanAspectState m_invitationalWhiteRequested = BooleanAspectState::Off;

    /**
     * This one may be set to On at any point in time, but it only lits up when m_mainAspectState
     * is Red. In that case, main aspect Red is not displayed. In other words, the shunting White
     * is suppressed by any main aspect state except Red. Red is itself suppressed by shunting
     * White.
     */
    BooleanAspectState m_shuntingWhiteRequested = BooleanAspectState::Off;

    SignalStateRuSymbolDisplay m_symbolDisplays;
};
