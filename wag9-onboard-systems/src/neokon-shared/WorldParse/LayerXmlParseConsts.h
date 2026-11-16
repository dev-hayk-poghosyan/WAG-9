#pragma once

/**
 * Contains names of the layer XML file items.
 * Used for parsing the layer XML file.
 */
class LayerXmlParseConsts
{
public:
    static const char* const nodeIdAttributeName();
    static const char* const nodeNameAttributeName();
    static const char* const nodeParentAttributeName();
    static const char* const transformNodeName();
    static const char* const nodesRootNodeName();
    static const char* const propertiesRootNodeName();
    static const char* const propertyNodeName();
    static const char* const parameterNodeName();
    static const char* const nodeName();
};
