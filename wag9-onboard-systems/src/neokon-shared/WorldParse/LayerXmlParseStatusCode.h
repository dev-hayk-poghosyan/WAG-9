#pragma once

#include <string>

/**
 * The code of the situation that occurred during the the parsing of a layer XML file. 
 */
enum class LayerXmlParseStatusCode
{
    Success = 0,
    CannotParseXml = 1,
    CannotFindRootInXml = 2,
    PropertyGuidNotFound = 3,
    NodeTransformNotFound = 4,
    NodeIdNotFound = 5,
    FileNotFound = 6,
    NodeNameNotFound = 7,
    PropertiesRootNotFound = 8,
    PropertyNotFound = 9,
    PropertyParameterNotFound = 10,
    NodeNotFound = 11
};

