#pragma once

#include "SignalState.h"
#include "SignalStateAspectBot.h"
#include "UkAplsState.h"
#include "UkJunctionIndicatorArm.h"
#include "UkMainAspectState.h"
#include "UkRightAwayState.h"

#include <optional>
#include <string>

/**
 * Represents the state of a 3 or 4-aspect signal with optional add-ons on the same pole.
 */
class SignalStateUkComposite : public SignalState, private SignalStateAspectBot
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
         * Why haven't we wrapped mainAspectState into an extra boost::optional?
         * Because this field being optional in the outer class is something we
         * had to do rather than something we wanted to do. To minimise the extra
         * complexity that brings, we haven't wrapped it into an extra boost::optional
         * here. After all, we don't want a partial state update being able to set
         * the main aspect state to null.
         *
         * All other fields we do wrap into an extra boost::optional.
         */
        std::optional<UkMainAspectState> mainAspectState;

        std::optional<std::optional<UkMainAspectState>> manualMainAspectState;

        std::optional<UkAplsState> manualAplsState;

        std::optional<UkRightAwayState> manualRightAwayState;

        std::optional<std::optional<UkJunctionIndicatorArm>> litJunctionIndicatorArm;

        std::optional<int> symbolicIndicator;

        std::optional<std::optional<int>> manualSymbolicIndicator;

        PartialState() = default;

        explicit PartialState(std::string const& serialized);

        std::string serialize() const;
    };

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

    virtual SignalStateAspectBot const* botAspect() const { return this; }

    std::optional<UkMainAspectState> mainAspectState() const { return m_mainAspectState; }

    void setMainAspectState(std::optional<UkMainAspectState> state) { m_mainAspectState = state; }

    std::optional<UkMainAspectState> manualMainAspectState() const {
        return m_manualMainAspectState;
    }

    void setManualMainAspectState(std::optional<UkMainAspectState> state) {
        m_manualMainAspectState = state;
    }

    UkAplsState manualAplsState() const { return m_manualAplsState; }

    void setManualAplsState(UkAplsState state) { m_manualAplsState = state; }

    UkRightAwayState manualRightAwayState() const { return m_manualRightAwayState; }

    std::optional<int> symbolicIndicator() const { return m_symbolicIndicator; }

    std::optional<int> manualSymbolicIndicator() const { return m_manualSymbolicIndicator; }

    void setManualRightAwayState(UkRightAwayState state) { m_manualRightAwayState = state; }

    std::optional<UkJunctionIndicatorArm> litJunctionIndicatorArm() const {
        return m_litJunctonIndicatorArm;
    }

    void setLitJunctionIndicatorArm(std::optional<UkJunctionIndicatorArm> litArm) {
        m_litJunctonIndicatorArm = litArm;
    }

private:
    virtual CommonCabinSignalColor commonCabinSignalColor() const override;

    /**
     * The actual main aspect state, determined by TrainSim.
     *
     * Why did we make this state optional? Because in RailroadControl, we want to display
     * m_manualMainAspectState when TrainSim is not there to set m_mainAspectState.
     */
    std::optional<UkMainAspectState> m_mainAspectState;

    /**
     * The main aspect state requested by the instructor.
     *
     * TrainSim does consult this value when setting m_mainAspectState, but it may still
     * set it to a stricter (and therefore safer) state than requested by the instructor.
     *
     * When set to boost::none, it means the signal light is in a fully automatic mode
     * (in the context of main aspect only).
     */
    std::optional<UkMainAspectState> m_manualMainAspectState;

    /**
     * The manual APLS state, set by the instructor.
     *
     * Whatever this value is set to, the APLS signal will not display anything unless
     * m_mainAspectState is Red.
     *
     * @note If this signal doesn't have an APLS add-on, the initial value shall never change.
     */
    UkAplsState m_manualAplsState = UkAplsState::ObeyMainSignal;

    /**
     * The manual RightAway (RA) signal state, set by the instructor.
     *
     * Even if set to On by the instructor, the signal will not display anything
     * if m_mainAspectState is Red or Off.
     *
     * @note If this signal doesn't have a RightAway add-on, the initial value shall never change.
     */
    UkRightAwayState m_manualRightAwayState = UkRightAwayState::Off;

    /**
     * The junction indicator arm that's lit, if any.
     *
     * Whatever this value is set to, the junction indicator will be off,
     * if m_mainAspectState is Red or Off (broken).
     *
     * This value is determined by TrainSim and the instructor can't modify it.
     */
    std::optional<UkJunctionIndicatorArm> m_litJunctonIndicatorArm;

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
