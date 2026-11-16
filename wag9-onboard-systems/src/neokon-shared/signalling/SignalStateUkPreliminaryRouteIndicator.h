#pragma once

#include "SignalState.h"
#include "UkPreliminaryRouteIndicatorState.h"

#include <optional>
#include <string>

/**
 * Represents the state of Preliminary Route Indicator signal which repeats the direction of the
 * junction indicator, shows up arrow or shows nothing.
 */
class SignalStateUkPreliminaryRouteIndicator : public SignalState
{
public:
    /**
     * Contains the same fields as the outer class, but wrapped into an extra boost::optional.
     * boost::none is interpreted as "the partial state doesn't carry this particular field".
     */
    struct PartialState
    {
        std::optional<UkPreliminaryRouteIndicatorState> indicatorState;

        PartialState() = default;

        explicit PartialState(std::string const& serialized);

        std::string serialize() const;
    };

    virtual std::unique_ptr<SignalState> clone() const override;

    virtual bool mergeSerializedPartialState(std::string const& serialized) override;

    /**
     * Takes the fields from the partial state that are set, and writes
     * them to the corresponding fields in this object.
     *
     * Returns true if any fields of this object changed their values.
     */
    bool mergePartialState(PartialState const& partialState);

    UkPreliminaryRouteIndicatorState indicatorState() const { return m_indicatorState; }

    void setState(UkPreliminaryRouteIndicatorState state) { m_indicatorState = state; }

private:
    /**
     * This state is determined by TrainSim and can't be altered by the instructor.
     */
    UkPreliminaryRouteIndicatorState m_indicatorState =
        UkPreliminaryRouteIndicatorState::NoInidicator;
};
