#pragma once

#include "LegacySignalLightState.h"
#include "SignalState.h"
#include "SignalStateAspectBot.h"
#include "SignalStateAspectRu.h"
#include "SignalStateAspectRuMetro.h"

/**
 * Same as SignalStateLegacy, but has a separate "manually requested state", which
 * may or may not override the automatic state.
 *
 * This class is not to be used for new signal lights. It exists because it allowed to
 * save time when transitioning to the new system of polymorphic signal states.
 */
class SignalStateLegacySemiAuto
    : public SignalState
    , private SignalStateAspectRu
    , private SignalStateAspectRuMetro
    , private SignalStateAspectBot
{
public:
    /**
     * Contains the same fields as the outer class, but some of them are wrapped into an extra
     * boost::optional. The outer boost::optional being null is always interpreted as
     * "the partial state doesn't carry this particular field".
     */
    struct PartialState
    {
        /**
         * Why haven't we wrapped actualState into an extra boost::optional?
         * Because this field being optional in the outer class is something we
         * had to do rather than something we wanted to do. To minimise the extra
         * complexity that brings, we haven't wrapped it into an extra boost::optional
         * here. After all, we don't want a partial state update being able to set
         * the actual signal state to null.
         *
         * All other fields we do wrap into an extra boost::optional.
         *
         * @see SignalStateLegacySemiAuto::m_actualState
         */
        std::optional<LegacySignalLightState> actualState;

        std::optional<std::optional<LegacySignalLightState>> manualStateRequest;

        PartialState() = default;

        explicit PartialState(std::string const& serialized);

        std::string serialize() const;
    };

    /**
     * Returns the partial state containing all the data from the full state.
     */
    PartialState toPartialState() const;

    virtual std::unique_ptr<SignalState> clone() const override;

    virtual bool mergeSerializedPartialState(std::string const& serialized);

    /**
     * Takes the fields from the partial state that are set, and writes
     * them to the corresponding fields in this object.
     *
     * Returns true if any fields of this object changed their values.
     */
    bool mergePartialState(PartialState const& partialState);

    virtual SignalStateAspectRu const* ruAspect() const override { return this; }

    virtual SignalStateAspectRuMetro const* ruMetroAspect() const override { return this; }

    virtual SignalStateAspectBot const* botAspect() const override { return this; }

    std::optional<LegacySignalLightState> actualState() const { return m_actualState; }

    void setActualState(std::optional<LegacySignalLightState> state) { m_actualState = state; }

    std::optional<LegacySignalLightState> manualStateRequest() const {
        return m_manualStateRequest;
    }

    void setManualStateRequest(std::optional<LegacySignalLightState> manualStateRequest) {
        m_manualStateRequest = manualStateRequest;
    }

    virtual bool noFrequency() const override;

    virtual bool absoluteStop() const override;

    virtual RuMetroAspectState aspectState() const override;

private:
    virtual CommonCabinSignalColor commonCabinSignalColor() const override;

    virtual RuMetroAlsCode alsCode() const override;

    virtual bool invitationActive() const override;

    /**
     * Why did we make the actual signal's state optional? Because that way, we can display
     * the manual state if the actual state is not set. This allows us to see the signals
     * displaying the state we've choosen manually, until a connection to TrainSim is established
     * and it sets whatever the actual state it wants (taking our manual state request into account).
     */
    std::optional<LegacySignalLightState> m_actualState;

    std::optional<LegacySignalLightState> m_manualStateRequest;
};
