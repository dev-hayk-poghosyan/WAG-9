#pragma once

#include "SignalState.h"
#include "UkBannerRepeaterState.h"

#include <optional>
#include <string>

/**
 * Represents the state of one or two banner repeaters on the same pole.
 */
class SignalStateUkBannerRepeater : public SignalState
{
public:
    /**
     * Contains the same fields as the outer class, but wrapped into an extra boost::optional.
     * boost::none is interpreted as "the partial state doesn't carry this particular field".
     */
    struct PartialState
    {
        std::optional<UkBannerRepeaterState> primaryBannerState;
        std::optional<UkBannerRepeaterState> secondaryBannerState;

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

    UkBannerRepeaterState primaryBannerState() const { return m_primaryBannerState; }

    void setPrimaryBannerState(UkBannerRepeaterState state) { m_primaryBannerState = state; }

    UkBannerRepeaterState secondaryBannerState() const { return m_secondaryBannerState; }

    void setSecondaryBannerState(UkBannerRepeaterState state) { m_secondaryBannerState = state; }

private:
    /**
     * The state of the upper (in two-banner configurations) or the only banner on a pole.
     *
     * This state is determined by TrainSim and can't be altered by the instructor.
     *
     * In two-banner configurations, this banner corresponds to the straight route.
     */
    UkBannerRepeaterState m_primaryBannerState = UkBannerRepeaterState::On;

    /**
     * The state of the lower banner on a pole.
     *
     * This state is determined by TrainSim and can't be altered by the instructor.
     *
     * In single-banner configurations, the default state is never changed.
     */
    UkBannerRepeaterState m_secondaryBannerState = UkBannerRepeaterState::On;
};
