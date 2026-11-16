#include "SignalStateUkSplittingDistant.h"

#include "SignalStateUkSplittingDistant.pb.h"

#include "neokon-shared/PlusEnum.h"

SignalStateUkSplittingDistant::PartialState
SignalStateUkSplittingDistant::toPartialState() const
{
    PartialState partialState;
    partialState.mainSectionsState = m_mainSectionsState;
    partialState.redStateRequestedManually = m_redStateRequestedManually;

    return partialState;
}

std::unique_ptr<SignalState>
SignalStateUkSplittingDistant::clone() const
{
    return std::make_unique<SignalStateUkSplittingDistant>(*this);
}

bool
SignalStateUkSplittingDistant::mergeSerializedPartialState(std::string const& serialized)
{
    return mergePartialState(PartialState(serialized));
}

bool
SignalStateUkSplittingDistant::mergePartialState(PartialState const& partialState)
{
    bool modified = false;

    if (partialState.mainSectionsState &&
        *partialState.mainSectionsState != m_mainSectionsState)
    {
        m_mainSectionsState = *partialState.mainSectionsState;
        modified = true;
    }

    if (partialState.redStateRequestedManually &&
        *partialState.redStateRequestedManually != m_redStateRequestedManually)
    {
        m_redStateRequestedManually = *partialState.redStateRequestedManually;
        modified = true;
    }

    return modified;
}

SignalStateUkSplittingDistant::MainSectionsState
SignalStateUkSplittingDistant::effectiveMainSectionsState() const
{
    if (m_mainSectionsState)
    {
        return *m_mainSectionsState;
    }
    else if (m_redStateRequestedManually)
    {
        return { UkMainAspectState::Red };
    }
    else
    {
        return {};
    }
}

CommonCabinSignalColor
SignalStateUkSplittingDistant::commonCabinSignalColor() const
{
    // This method is used by bot trains.

    if (m_mainSectionsState)
    {
        switch (m_mainSectionsState->rightSectionState)
        {
        case UkMainAspectState::Green:
            return CommonCabinSignalColor::Green;
        case UkMainAspectState::Yellow:
        case UkMainAspectState::DoubleYellow:
        case UkMainAspectState::FlashingYellow:
        case UkMainAspectState::FlashingDoubleYellow:
            return CommonCabinSignalColor::Yellow;
        case UkMainAspectState::Red:
            return CommonCabinSignalColor::RedYellow;
        case UkMainAspectState::Off:
            // This means the states are specified for the straight and diverging sections.
            break;
        }

        // Note that m_mainSectionsState->rightSectionState being Off already implies
        // that the route to the junction signal is clear and that the junction signal
        // itself is not red. Therefore, below we decide between Green, Yellow and Off.

        if (m_mainSectionsState->straightSectionState == UkMainAspectState::Green ||
            m_mainSectionsState->divergingSectionState == UkMainAspectState::Green)
        {
            // If one of the sections shows Green and the other shows something else, that
            // necessarily means the route goes towards the one that shows Green.
            return CommonCabinSignalColor::Green;
        }

        if (m_mainSectionsState->straightSectionState != UkMainAspectState::Off ||
            m_mainSectionsState->divergingSectionState != UkMainAspectState::Off)
        {
            return CommonCabinSignalColor::Yellow;
        }
    }

    return CommonCabinSignalColor::Off;
}


/*============================= MainSectionStates ========================*/

bool operator==(
    SignalStateUkSplittingDistant::MainSectionsState const& lhs,
    SignalStateUkSplittingDistant::MainSectionsState const& rhs)
{
    return lhs.rightSectionState == rhs.rightSectionState &&
        lhs.straightSectionState == rhs.straightSectionState &&
        lhs.divergingSectionState == rhs.divergingSectionState;
}

bool operator!=(
    SignalStateUkSplittingDistant::MainSectionsState const& lhs,
    SignalStateUkSplittingDistant::MainSectionsState const& rhs)
{
    return !(lhs == rhs);
}

/*============================== PartialState ============================*/

SignalStateUkSplittingDistant::PartialState::PartialState(std::string const& serialized)
{
    proto::SignalStateUkSplittingDistant protoState;
    if (!protoState.ParseFromString(serialized))
    {
        return;
    }

    if (protoState.optional_main_sections_state_case() == protoState.kMainSectionsState)
    {
        auto const& protoSectionsState = protoState.main_sections_state();

        UkMainAspectState const rightSectionState = ukMainAspectStateFromInt(
            protoSectionsState.right_section_state()).value_or(UkMainAspectState::Off);

        UkMainAspectState const straightSectionState = ukMainAspectStateFromInt(
            protoSectionsState.straight_section_state()).value_or(UkMainAspectState::Off);

        UkMainAspectState const divergingSectionState = ukMainAspectStateFromInt(
            protoSectionsState.diverging_section_state()).value_or(UkMainAspectState::Off);

        mainSectionsState = MainSectionsState{
            rightSectionState,
            straightSectionState,
            divergingSectionState
        };
    }

    if (protoState.optional_red_state_requested_manually_case() ==
        protoState.kRedStateRequestedManually)
    {
        redStateRequestedManually = protoState.red_state_requested_manually();
    }
}

std::string
SignalStateUkSplittingDistant::PartialState::serialize() const
{
    proto::SignalStateUkSplittingDistant protoState;

    if (mainSectionsState)
    {
        auto& protoSectionsState = *protoState.mutable_main_sections_state();

        protoSectionsState.set_right_section_state(+mainSectionsState->rightSectionState);
        protoSectionsState.set_straight_section_state(+mainSectionsState->straightSectionState);
        protoSectionsState.set_diverging_section_state(+mainSectionsState->divergingSectionState);
    }

    if (redStateRequestedManually)
    {
        protoState.set_red_state_requested_manually(*redStateRequestedManually);
    }

    std::string serialized;
    protoState.SerializeToString(&serialized);

    return serialized;
}
