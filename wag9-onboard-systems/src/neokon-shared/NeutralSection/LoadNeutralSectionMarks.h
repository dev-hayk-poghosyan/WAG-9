#pragma once

#include "NeutralSectionMark.h"
#include "NeutralSectionMarksLoadingStatus.h"

#include <filesystem>
#include <vector>

class NeutralSectionConsts
{
public:

    static const char* neutralSectionParentName();
    static std::string propertiesDirName();
    static std::string neutralSectionPropFileName();
    static std::string neutralSectionPropParamName();
    static std::string neutralSectionPropActiveParamName();
};

/**
 * Loads neutral section marks from the layer file. @see documentation to NeutralSectionMark.
 */
NeutralSectionMarksLoadingStatus loadNeutralSectionMarks(std::string const& worldDirName,
    std::string const& terrainEditorDataPath, std::string const& layerFileName,
    std::vector<NeutralSectionMark>& result);
