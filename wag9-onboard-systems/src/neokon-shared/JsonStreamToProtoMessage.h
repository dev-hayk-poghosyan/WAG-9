#pragma once

#include <google/protobuf/message.h>

#include <iosfwd>

/**
 * @throw std::exception If anything goes wrong.
 */
void jsonStreamToProtoMessage(std::istream& strm, google::protobuf::Message& msg);
