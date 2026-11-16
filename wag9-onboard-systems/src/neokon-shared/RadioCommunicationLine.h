#pragma once

#include "RadioAudioLine.pb.h"

/**
 * Enumeration of integrated communication lines.
 */
enum class RadioCommunicationLine
{
    /**
     * GSM-R wireless standard communication for railway.
     */
    Gsmr = proto::radio::AudioLine::GSMR,

    /**
     * All local communications between cabs, public address etc.
     */
    Train = proto::radio::AudioLine::TRAIN,

    /**
     * Informal communication between instructor and student.
     */
    Direct = proto::radio::AudioLine::DIRECT,
};

inline RadioCommunicationLine
radioCommLineFromProto(proto::radio::AudioLine::Line line)
{
    return static_cast<RadioCommunicationLine>(line);
}

inline proto::radio::AudioLine::Line
protoFromRadioCommLine(RadioCommunicationLine line)
{
    return static_cast<proto::radio::AudioLine::Line>(line);
}

const char* radioCommunicationLineToString(RadioCommunicationLine line);
