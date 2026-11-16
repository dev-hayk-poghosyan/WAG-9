#pragma once

#include <google/protobuf/message.h>

#include <filesystem>

void simpleEncryptProto(google::protobuf::Message& msg, std::filesystem::path const& filePath);

void simpleDecryptProto(std::filesystem::path const& filePath, google::protobuf::Message& msg);
