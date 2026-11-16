#pragma once

#include "TrainPlatformSituationType.pb.h"
#include "PlusEnum.h"

/**
 * Available train platform situation types.
 */
enum class TrainPlatformSituationType : int
{
    Empty = 0,
    CoatInTheDoor = 1,
    FallingHuman = 2,
    Fight = 3,
    PersonFeelsBad = 4,
    DisabledPerson = 5,
    PersonSittingOnTheEdge = 6,
    Leans = 7
};

inline TrainPlatformSituationType platformSituationTypeFromProto(
    proto::TrainPlatformSituationType::Type situationType)
{
    static_assert(+TrainPlatformSituationType::Empty == proto::TrainPlatformSituationType::EMPTY);
    static_assert(+TrainPlatformSituationType::Fight == proto::TrainPlatformSituationType::FIGHT);
    static_assert(+TrainPlatformSituationType::CoatInTheDoor ==
        proto::TrainPlatformSituationType::COAT_IN_THE_DOOR);
    static_assert(+TrainPlatformSituationType::DisabledPerson ==
        proto::TrainPlatformSituationType::DISABLED_PERSON);
    static_assert(+TrainPlatformSituationType::FallingHuman ==
        proto::TrainPlatformSituationType::FALLING_HUMAN);
    static_assert(+TrainPlatformSituationType::PersonFeelsBad ==
        proto::TrainPlatformSituationType::PERSON_FEELS_BAD);
    static_assert(+TrainPlatformSituationType::PersonSittingOnTheEdge ==
        proto::TrainPlatformSituationType::PERSON_SITTING_ON_THE_EDGE);
    static_assert(+TrainPlatformSituationType::Leans ==
        proto::TrainPlatformSituationType::LEANS);

    return static_cast<TrainPlatformSituationType>(situationType);
}

inline proto::TrainPlatformSituationType::Type platformSituationTypeToProto(
    TrainPlatformSituationType situationType)
{
    static_assert(+TrainPlatformSituationType::Empty == proto::TrainPlatformSituationType::EMPTY);
    static_assert(+TrainPlatformSituationType::Fight == proto::TrainPlatformSituationType::FIGHT);
    static_assert(+TrainPlatformSituationType::CoatInTheDoor ==
        proto::TrainPlatformSituationType::COAT_IN_THE_DOOR);
    static_assert(+TrainPlatformSituationType::DisabledPerson ==
        proto::TrainPlatformSituationType::DISABLED_PERSON);
    static_assert(+TrainPlatformSituationType::FallingHuman ==
        proto::TrainPlatformSituationType::FALLING_HUMAN);
    static_assert(+TrainPlatformSituationType::PersonFeelsBad ==
        proto::TrainPlatformSituationType::PERSON_FEELS_BAD);
    static_assert(+TrainPlatformSituationType::PersonSittingOnTheEdge ==
        proto::TrainPlatformSituationType::PERSON_SITTING_ON_THE_EDGE);
    static_assert(+TrainPlatformSituationType::Leans ==
        proto::TrainPlatformSituationType::LEANS);

    return static_cast<proto::TrainPlatformSituationType::Type>(situationType);
}

inline std::string enumToString(TrainPlatformSituationType situationType)
{
    std::string result;
    switch (situationType)
    {
    case TrainPlatformSituationType::CoatInTheDoor:
        result = "CoatInTheDoor";
        break;
    case TrainPlatformSituationType::Empty:
        result = "Empty";
        break;
    case TrainPlatformSituationType::FallingHuman:
        result = "PassengerDrop";
        break;
    case TrainPlatformSituationType::Fight:
        result = "Fight";
        break;
    case TrainPlatformSituationType::DisabledPerson:
        result = "DisabledPerson";
        break;
    case TrainPlatformSituationType::PersonFeelsBad:
        result = "PersonFeelsBad";
        break;
    case TrainPlatformSituationType::PersonSittingOnTheEdge:
        result = "PersonSittingOnTheEdge";
        break;
    case TrainPlatformSituationType::Leans:
        result = "Leans";
        break;
    default:
        throw std::logic_error("Invalid platform situation type");
    }
    return result;
}
