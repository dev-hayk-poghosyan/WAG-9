#pragma once

#include "NeutralSectionMarksLoadingStatusCode.h"

#include <string>
#include <vector>

/**
 * Contains the code and parameters of the situation that occurred during the loading of the
 * neutral section mark from the layer file. @see documentation to NeutralSectionMark and
 * loadNeutralSectionMarks.
 *
 * TODO: change to LayerXmlParseStatus.
 */
struct NeutralSectionMarksLoadingStatus
{
    NeutralSectionMarksLoadingStatus(NeutralSectionMarksLoadingStatusCode aStatusCode,
        std::string const& aFilePath, std::string const& aNodeName, std::string const& anErrorMsg)
        : statusCode(aStatusCode)
        , filePath(aFilePath)
        , nodeName(aNodeName)
        , errorMsg(anErrorMsg)
    {
    }

    const NeutralSectionMarksLoadingStatusCode statusCode;
    const std::string filePath;
    const std::string nodeName;
    const std::string errorMsg;
};
