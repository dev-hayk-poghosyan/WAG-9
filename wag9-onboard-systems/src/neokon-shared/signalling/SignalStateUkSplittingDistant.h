#pragma once

#include "SignalState.h"
#include "SignalStateAspectBot.h"
#include "UkMainAspectState.h"

#include <optional>
#include <string>

/**
 * Represents the state of splitting distant siagnal, that has 2 main aspect signals:
 * the one for the straight route (above) and the one for the diverging route (below
 * and horizontally shifted to the appropriate side).
 */
class SignalStateUkSplittingDistant : public SignalState, private SignalStateAspectBot
{
public:
    struct MainSectionsState
    {
        /**
         * If rightSectionState is not Off, then the right section uses this value and
         * the left section is assumed to be Off. In such a case, @p straightSectionState
         * and @p divergingSectionState are not used, and we require them to be set to Off.
         * This situation implies that either the junction signal ahead is Red or that
         * there is a road block before the junction signal. In such a scenario, only the
         * right section is used, presumably because it's the one closer to the driver.
         *
         * If rightSectionState is Off, then the section's states are taken from
         * @p straightSectionState and @p divergingSectionState. This situation implies
         * that the route to the junction signal is clear and that the junction signal
         * is not Red.
         */
        UkMainAspectState rightSectionState = UkMainAspectState::Off;

        UkMainAspectState straightSectionState = UkMainAspectState::Off;
        UkMainAspectState divergingSectionState = UkMainAspectState::Off;

        friend bool operator==(MainSectionsState const& lhs, MainSectionsState const& rhs);

        friend bool operator!=(MainSectionsState const& lhs, MainSectionsState const& rhs);
    };

    /**
     * Contains the same fields as the outer class, but some of them are wrapped into an extra
     * std::optional. The outer std::optional being null is always interpreted as "the partial
     * state doesn't carry this particular field.
     */
    struct PartialState
    {
        /**
         * Why haven't we wrapped mainSectionsState into an extra std::optional?
         * Because this field being optional in the outer class is something we
         * had to do rather than something we wanted to do. To minimise the extra
         * complexity that brings, we haven't wrapped it into an extra std::optional
         * here. After all, we don't want a partial state update being able to set
         * the state of the main sections to null.
         */
        std::optional<MainSectionsState> mainSectionsState;

        std::optional<bool> redStateRequestedManually;

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

    std::optional<MainSectionsState> const& mainSectionsState() const {
        return m_mainSectionsState;
    }

    /**
     * If mainSectionsState() returns a non-null value, this method returns that value.
     * Otherwise it returns the manually requested state, if there is one. If not,
     * it returns a state where both main sections are off.
     */
    MainSectionsState effectiveMainSectionsState() const;

    bool redStateRequestedManually() const { return m_redStateRequestedManually; }

private:
    virtual CommonCabinSignalColor commonCabinSignalColor() const override;

    /**
     * The actual state of both the straight and the diverging route sections,
     * determined by TrainSim.
     *
     * Why did we make this state optional? Because in RailroadControl, we want to display
     * the manually requested state when TrainSim is not there to set the actual one.
     */
    std::optional<MainSectionsState> m_mainSectionsState;

    /**
     * When set to true, that means the instructor wants m_mainSectionsState->redState
     * to be true. The whole m_mainSectionsState is controlled by TrainSim, so it's up
     * to TrainSim to take the instructor's request into account.
     */
    bool m_redStateRequestedManually = false;
};
