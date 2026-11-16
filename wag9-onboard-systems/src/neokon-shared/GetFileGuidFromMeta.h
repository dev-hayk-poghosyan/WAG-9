#pragma once

#include <boost/filesystem/path.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <filesystem>
#include <string>

/**
* @returns the GUID of the property specified by @p propertyRelFilePath which is relative
* to terrain editor data path.
*/
std::string getFileGuidFromMeta(
    std::filesystem::path const& terrainEditorDataAbsDirPath,
    std::filesystem::path  const& propertyRelFilePath);
