#pragma once

/**
 * The code of the situation that occurred during the loading of the neutral section mark
 * from the layer file. @see documentation to NeutralSectionMark and loadNeutralSectionMarks.
 *
 * TODO: change to LayerXmlParseStatusCode.
 */
enum class NeutralSectionMarksLoadingStatusCode
{
    Success = 0,
    CannotParseXml = 1,
    CannotFindRootInXml = 2,
    PropertyGuidNotFound = 3,
    NodeTransformNotFound = 4,
    NodeIdNotFound = 5,
    FileNotFound = 6,
    NodeNameNotFound = 7
};
