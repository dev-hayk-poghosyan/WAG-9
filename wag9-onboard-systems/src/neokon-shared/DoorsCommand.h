#pragma once

#include "PlusEnum.h"
#include "DoorsCommand.pb.h"

enum class DoorsCommand
{
    OpenLeftDoors = proto::DoorsCommand::OPEN_LEFT_DOORS,
    OpenRightDoors = proto::DoorsCommand::OPEN_RIGHT_DOORS,
    CloseAllDoors = proto::DoorsCommand::CLOSE_ALL_DOORS,
    ConductorLeft = proto::DoorsCommand::CONDUCTOR_LEFT,
    ConductorRight = proto::DoorsCommand::CONDUCTOR_RIGHT,
    InterlockClose = proto::DoorsCommand::INTERLOCK_CLOSE,
};

inline DoorsCommand doorsCommandFromProto(proto::DoorsCommand const& doorsCommand)
{
    return static_cast<DoorsCommand>(doorsCommand);
}

inline proto::DoorsCommand protoFromDoorsCommand(DoorsCommand const& doorsCommand)
{
    return static_cast<proto::DoorsCommand>(doorsCommand);
}
