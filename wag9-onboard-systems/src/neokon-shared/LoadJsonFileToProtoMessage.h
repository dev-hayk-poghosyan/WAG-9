#pragma once

#include <google/protobuf/message.h>

#include <filesystem>

bool tryLoadJsonFileToProtoMessage(
    std::filesystem::path const& filePath, google::protobuf::Message& msg);

/**
 * @throw std::exception If anything goes wrong.
 */
void loadJsonFileToProtoMessage(
    std::filesystem::path const& filePath, google::protobuf::Message& msg);
