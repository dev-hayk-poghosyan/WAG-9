#pragma once

#include "RuMetroAspectState.h"
#include "SignalRuMetroSpeedLimit.h"

#include "neokon-shared/signalling/RuSymbolDisplayState.h"
#include "neokon-shared/signalling/BooleanAspectState.h"
#include "neokon-shared/signalling/CommonCabinSignalColor.h"
#include "neokon-shared/signalling/LampMode.h"
#include "neokon-shared/signalling/SignalSpeedLimit.h"
#include "neokon-shared/signalling/SignalState.h"
#include "neokon-shared/signalling/SignalStateAspectBot.h"
#include "neokon-shared/signalling/SignalStateAspectRuMetro.h"
#include "neokon-shared/signalling/SignalStateRuSymbolDisplay.h"

#include <optional>

class SignalStateRuMetro
    : public ContainsRuSymbolDisplayState
    , public SignalState
    , private SignalStateAspectBot
    , private SignalStateAspectRuMetro
{
public:
    struct PartialState
    {
        PartialState() = default;

        explicit PartialState(std::string const& serialized);

        /**
         * Methods to convert proto data
         * to Signal state fields for deserialization
         */
        std::optional<RuMetroAspectState> fromIntToAspectState(int state) const;
        void fromProto(int protoVal, std::optional<RuMetroAspectState>& target) const;
        void fromProto(int protoVal, std::optional<std::optional<RuMetroAspectState>>& target) const;
        void fromProto(int protoVal, std::optional<std::optional<RuSymbolDisplayState>>& target) const;
        void fromProto(int protoVal, std::optional<bool>& target) const;
        void fromProto(int protoVal, std::optional<RuMetroAlsCode>& target) const;
        /**
         * Methods to convert Signal state fields
         * to proto data for serialization
         */
        int toProto(bool value) const;
        int toProto(std::optional<bool> const& value) const;
        int toProto(RuMetroAspectState value) const;
        int toProto(std::optional<RuMetroAspectState> const& value) const;
        int toProto(std::optional<std::optional<RuMetroAspectState>> const& value) const;
        int toProto(std::optional<RuSymbolDisplayState> const& value) const;
        int toProto(std::optional<RuMetroAlsCode> const& value) const;

        std::string serialize() const;

        /**
         * Sets symbol display state.
         * @param index: 0 - first display, 1 - second display.
         * @param value: diplay symbol mnemonics.
         * Returns true if display state was changed.
         */
        bool setSymbolDisplayState(unsigned int index, RuSymbolDisplayState value);
        /**
         * Sets symbol display state KGU.
         * @param value: TRUE - show, FALSE - hide.
         * Returns true if display state was changed.
         */
        bool setSymbolDisplayStateKGU(bool value);

        std::optional<RuMetroAlsCode> m_alsFrequency;

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
         * @see SignalStateRuMetro::m_mainAspectState
         */
        std::optional<RuMetroAspectState> m_mainAspectState;

        std::optional<std::optional<RuMetroAspectState>> m_manuallyRequestedMainAspectState;

        std::optional<bool> m_autoBlocking;

        std::optional<bool> m_absoluteStop;

        SignalStateRuSymbolDisplay::PartialState symbolDisplays;
    };

    static CommonCabinSignalColor commonCabinSignalColor(RuMetroAspectState state);

    static std::vector<RuMetroAspectState> parseAspectState(const std::string& state);

    /**
     * Returns the partial state containing all the data from the full state.
     */
    PartialState toPartialState() const;

    bool kguDisplayState() const;

    //override for ContainsRuSymbolDisplayState
    virtual std::optional<bool> kguDispayValue() const override;
    virtual RuSymbolDisplayState symbolDispayValue(unsigned int index) const override;

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

    virtual SignalStateRuMetro const* ruMetroAspect() const override { return this; }

    bool autoBlockingState() const { return m_autoBlocking; }

    std::optional<RuMetroAspectState> mainAspectState() const {
        return m_mainAspectState;
    }

    std::optional<RuMetroAspectState> manuallyRequestedMainAspectState() const {
        return m_manuallyRequestedMainAspectState;
    }

    /**
     * Returns mainAspectState() if it's set, or manuallyRequestedMainAspectState() otherwise.
     */
    std::optional<RuMetroAspectState> actualOrRequestedMainAspectState() const;

    bool isFlashingValue(RuMetroAspectState state) const;

    //override for SignalStateAspectBot
    virtual CommonCabinSignalColor commonCabinSignalColor() const override;

    //override for SignalStateAspectRuMetro
    virtual RuMetroAlsCode alsCode() const override;

    virtual bool invitationActive() const override;

    virtual bool noFrequency() const override;

    virtual bool absoluteStop() const override;

    virtual RuMetroAspectState aspectState() const override;

    std::string alsCodeStr() const;

    void setStateForAlsCode(RuMetroAspectState state) { m_alsState = state; }

    void setNoFrequencyMode() { m_noFrequency = true; }

    void setStandardFrequencyMode()
    {
        m_noFrequency = false;
        m_specFrequency = false;
    }

    void setSpecFrequencyMode()
    {
        m_noFrequency = false;
        m_specFrequency = true;
    }

    void setSpeedLimit(std::shared_ptr<SignalRuMetroSpeedLimit> const& limit)
    {
        m_limit = limit;
    }

private:
    /**
     * Special mode assumes any frequency from Red0 to Green80
     * according to the current aspect state.
     */
    RuMetroAlsCode alsCodeCommon(RuMetroAspectState state) const;

    /**
     * Special mode assumes frequency Yellow40 for all aspect states
     * give the train permission to move and Red0 otherwise.
     */
    RuMetroAlsCode alsCodeSpecial(RuMetroAspectState state) const;

    std::optional<RuMetroAlsCode> m_alsFrequency;

    /**
     * Why did we make the actual signal's state optional? Because that way, we can display
     * the manual state if the actual state is not set. This allows us to see the signals
     * displaying the state we've choosen manually, until a connection to TrainSim is established
     * and it sets whatever the actual state it wants (taking our manual state request into account).
     */
    std::optional<RuMetroAspectState> m_mainAspectState;

    std::optional<RuMetroAspectState> m_manuallyRequestedMainAspectState;

    std::shared_ptr<SignalRuMetroSpeedLimit> m_limit;

    SignalStateRuSymbolDisplay m_symbolDisplays;

    std::optional<RuMetroAspectState> m_alsState;

    bool m_autoBlocking = false;
    bool m_noFrequency = false;
    bool m_specFrequency = false;

    /**
     * For semi-auto signals.
     */
    bool m_absoluteStop = false;
};
