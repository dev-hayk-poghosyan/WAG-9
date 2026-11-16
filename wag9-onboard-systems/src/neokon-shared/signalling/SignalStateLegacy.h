#pragma once

#include "LegacySignalLightState.h"
#include "SignalState.h"
#include "SignalStateAspectBot.h"
#include "SignalStateAspectRu.h"
#include "SignalStateAspectRuMetro.h"

/**
 * This class is not to be used for new signal lights. It exists because it allowed to
 * save time when transitioning to the new system of polymorphic signal states.
 */
class SignalStateLegacy
    : public SignalState
    , private SignalStateAspectRu
    , private SignalStateAspectRuMetro
    , private SignalStateAspectBot
{
public:
    /**
     * The concept of a partial state is not strictly necessary here, as this state
     * only has one field, however it's present for consistency with more complex signal states.
     */
    struct PartialState
    {
        std::optional<LegacySignalLightState> legacyState;

        std::optional<int> symbolicIndicator;

        std::optional<std::optional<int>> manualSymbolicIndicator;

        PartialState() = default;

        explicit PartialState(std::string const& serialized);

        std::string serialize() const;
    };

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

    virtual bool noFrequency() const override;

    virtual bool absoluteStop() const override;

    virtual RuMetroAspectState aspectState() const override;

    LegacySignalLightState legacyState() const { return m_legacyState; }

    std::optional<int> symbolicIndicator() const { return m_symbolicIndicator; }

    std::optional<int> manualSymbolicIndicator() const { return m_manualSymbolicIndicator; }

private:
    virtual CommonCabinSignalColor commonCabinSignalColor() const override;

    virtual RuMetroAlsCode alsCode() const override;

    virtual bool invitationActive() const override;

    LegacySignalLightState m_legacyState = LegacySignalLightState::Off;

    /**
     * The actual symbolic indicator's node id that are shown in TrainSim.
     */
    std::optional<int> m_symbolicIndicator;

    /**
     * Requested from the instructor symbolic indicator's node id.
     * This request may be ignored by the Trainsim. Actual value will be in m_symbolicIndicator.
     */
    std::optional<int> m_manualSymbolicIndicator;
};
