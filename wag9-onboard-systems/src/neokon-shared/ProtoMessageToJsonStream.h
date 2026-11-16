#pragma once

#include <google/protobuf/message.h>
#include <google/protobuf/util/json_util.h>

#include <iosfwd>

/**
 * @throw std::exception If anything goes wrong.
 */
void protoMessageToJsonStream(google::protobuf::Message const& msg, std::ostream& strm);

/**
 * @throw std::exception If anything goes wrong.
 */
void protoMessageToJsonStream(google::protobuf::Message const& msg, std::ostream& strm,
    google::protobuf::util::JsonPrintOptions option);
