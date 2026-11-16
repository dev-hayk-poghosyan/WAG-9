#pragma once

#include "SwitcherMark.h"
#include "SupplyChangeWarningMark.h"

#include "neokon-shared/WorldParse/LayerXmlParseStatus.h"

#include <filesystem>
#include <memory>
#include <vector>

class UnigineGuidDatabase;

/**
 * Used to represent a result of loading supply change marks from a layer file.
 */
struct SupplyChangeMarksLoadingResult
{
    /**
     * Status of the XML parsing.
     */
    const std::vector<LayerXmlParseStatus> parseErrors;

    const std::vector<SwitcherMark> switcherMarks;

    const std::vector<DirectionMark> directionMarks;

    const std::vector<SupplyChangeWarningMark> warningMarks;
};

/**
 * Load supply change marks from the layer file represented by @p layerFilePath.
 * @see SupplyChangeMark.
 */
SupplyChangeMarksLoadingResult loadSupplyChangeMarks(const std::filesystem::path& layerFilePath,
    const std::filesystem::path& propsDirPath, const std::string& xmlRootNodeName,
    const std::shared_ptr<UnigineGuidDatabase>& guidDatabase);
