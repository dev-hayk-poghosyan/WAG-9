#pragma once

#include "PlusEnum.h"
#include "EtcsModeCommand.pb.h"

enum class EtcsModeCommand
{
    OS = proto::EtcsModeCommand::OS,
    SH = proto::EtcsModeCommand::SH,
    SR = proto::EtcsModeCommand::SR,
};

inline EtcsModeCommand etcsModeCommandFromProto(proto::EtcsModeCommand const& etcsModeCommand)
{
    return static_cast<EtcsModeCommand>(etcsModeCommand);
}

inline proto::EtcsModeCommand protoFromEtcsModeCommand(EtcsModeCommand const& etcsModeCommand)
{
    return static_cast<proto::EtcsModeCommand>(etcsModeCommand);
}
