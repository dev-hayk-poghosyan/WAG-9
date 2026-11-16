#include "LoadSupplyChangeMarks.h"

#include "SupplyChangeConsts.h"

#include "neokon-shared/UnigineGuidDatabase.h"
#include "neokon-shared/SupplyChangeMarks/SwitcherMark.h"
#include "neokon-shared/WorldParse/LayerXmlParseConsts.h"
#include "neokon-shared/WorldParse/XmlProperties.h"
#include "neokon-shared/WorldParse/XmlTransformToGlmMatrix.h"

#include <pugixml.hpp>

#include <algorithm>
#include <filesystem>
#include <numeric>
#include <optional>
#include <sstream>

namespace
{
/**
 * Represents a result of parsing mark node.
 */
struct MarkNodeParseResult
{
    /**
     * Loaded name.
     */
    const std::string name;

    /**
     * Loaded transform.
     */
    const glm::dmat4 transform;
};

/**
 * Contains property parameter values, extracted from "TrainPowerSupplySwitchWarning.prop".
 */
struct SupplyChangeWarningPropParams
{
    const std::string hostMarkName;
    const bool isAheadMark;
};

/**
 * @returns the root XML-node for all xml-nodes of the "node" type.
 * @throws LayerXmlParseException if the root node is not found.
 */
pugi::xml_node findNodesRoot(const pugi::xml_document& doc, const std::string& filename)
{
    const pugi::xml_node parentNode{ doc.child(LayerXmlParseConsts::nodesRootNodeName()) };
    return parentNode && !parentNode.empty()
        ? parentNode
        : throw LayerXmlParseException({ filename, LayerXmlParseStatusCode::CannotFindRootInXml });
}

/**
 * @returns the root XML-node for all supply change marks.
 * @throws LayerXmlParseException if the node is not found.
 */
pugi::xml_node findMarksRootNode(const pugi::xml_node& nodesRoot,
    const std::string& marksRootNodeName, const std::string& filename)
{
    const auto& container = nodesRoot.children(LayerXmlParseConsts::nodeName());
    const auto childrenIter = std::find_if(container.begin(), container.end(),
        [&marksRootNodeName](const pugi::xml_node& node)
    {
        return node.attribute(LayerXmlParseConsts::nodeNameAttributeName())
            ? std::string(node.attribute(
                LayerXmlParseConsts::nodeNameAttributeName()).as_string()) == marksRootNodeName
            : false;
    });

    return childrenIter != container.end() && *childrenIter && !childrenIter->empty()
        ? *childrenIter
        : throw LayerXmlParseException({ filename, marksRootNodeName,
            LayerXmlParseStatusCode::NodeNotFound });
}

/**
 * @returns the name of @p node.
 * @throws LayerXmlParseException if the name attribute is not found.
 */
std::string nodeName(const pugi::xml_node& node, const std::string& filename)
{
    const auto& nameAttr = node.attribute(LayerXmlParseConsts::nodeNameAttributeName());
    return nameAttr && !nameAttr.empty()
        ? nameAttr.as_string()
        : throw LayerXmlParseException({ filename, LayerXmlParseStatusCode::NodeNameNotFound });
}

/**
 * @returns the transform xml-node for the @p node.
 * @throws LayerXmlParseException if the transform node is not found.
 */
pugi::xml_node findTransformNode(const pugi::xml_node& node, const std::string& filename)
{
    const pugi::xml_node transformNode{ node.child(LayerXmlParseConsts::transformNodeName()) };
    return transformNode && !transformNode.empty()
        ? transformNode
        : throw LayerXmlParseException({ filename, nodeName(node, filename),
            LayerXmlParseStatusCode::NodeTransformNotFound });
}

/**
 * Calculates the transform matrix based on the @p parentTransform and @p nodeTransformXmlNode.
 */
const glm::dmat4 calcTransform(const glm::dmat4& parentTransform,
    const pugi::xml_node& nodeTransformXmlNode)
{
    return parentTransform * xmlTransformToGlmMatrix(nodeTransformXmlNode.text().as_string());
}

/**
 * @returns GUID of the @p propertyFilePath.
 * @throws LayerXmlParseException if the GUID is not found in the @p guidDatabase.
 */
const std::string findPropertyGuidInDatabse(const std::filesystem::path& propertyFilePath,
    const std::shared_ptr<UnigineGuidDatabase>& guidDatabase)
{
    const std::string propGuid{ guidDatabase->guid(propertyFilePath.string()) };
    return propGuid.empty()
        ? throw LayerXmlParseException({ propertyFilePath.string(),
            LayerXmlParseStatusCode::PropertyGuidNotFound })
        : propGuid;
}

/**
 * @returns the property XML-node with the @p propGuid of the @p node.
 * @throws LayerXmlParseException if the property is not found.
 */
const pugi::xml_node handleFindUniginePropertyError(const pugi::xml_node& node,
    const std::string& propGuid, const std::string& filename)
{
    const pugi::xml_node propNode{ findUnigineProperty(node, propGuid) };
    return propNode && !propNode.empty()
        ? propNode
        : throw LayerXmlParseException({ filename, nodeName(node, filename),
            LayerXmlParseStatusCode::PropertyNotFound });
}

/**
 * @returns property parameter XML-node with the @p paramName of the @p propNode.
 * @throws LayerXmlParseException if the parameter is not found.
 */
const pugi::xml_node handleFindUniginePropertyParameterError(const pugi::xml_node& propNode,
    const std::string& paramName, const std::string& filename)
{
    const pugi::xml_node paramNode{ findUniginePropertyParameter(propNode, paramName) };
    return paramNode && !paramNode.empty()
        ? paramNode
        : throw LayerXmlParseException({ filename, nodeName(propNode, filename), paramName,
            LayerXmlParseStatusCode::PropertyParameterNotFound });
}

/**
 * Looks for property represented by @p propGuid in the @p node and reads its parameter.
 */
bool supplyChangeMarkNodeActiveStatus(const pugi::xml_node& node,
    const std::string& propGuid, const std::string& filename)
{
    return handleFindUniginePropertyParameterError(findUnigineProperty(node, propGuid),
        SupplyChangeConsts::propertyParamName(), filename).text().as_bool();
}

/**
 * Looks for property represented by @p propGuid in the @p node and extracts its parameters.
 */
SupplyChangeWarningPropParams extractSupplyChangeWarningPropParams(const pugi::xml_node& node,
    const std::string& propGuid, const std::string& filename)
{
    const auto hostMarkParamNode = handleFindUniginePropertyParameterError(
        findUnigineProperty(node, propGuid), SupplyChangeConsts::hostMarkParamName(), filename);
    const auto isAheadParamNode = handleFindUniginePropertyParameterError(
        findUnigineProperty(node, propGuid), SupplyChangeConsts::isAheadMarkParamName(), filename);

    return SupplyChangeWarningPropParams{
        hostMarkParamNode.text().as_string(),
        isAheadParamNode.text().as_bool()
    };
}

/**
 * Parses the @p node and tries to get node name and node transform.
 */
MarkNodeParseResult parseMarkXmlNode(const pugi::xml_node& node, const glm::dmat4& parentTransform,
    const std::string& filename)
{
    return { nodeName(node,filename),
        calcTransform(parentTransform, findTransformNode(node, filename)) };
}

/**
 * Parses the @p node and tries to create a supply change mark from it.
 */
SwitcherMark processSupplyChangeMarkXmlNode(const pugi::xml_node& node,
    const glm::dmat4& parentTransform, const std::string& propGuid, const std::string& filename)
{
    const MarkNodeParseResult markParseResult{ parseMarkXmlNode(node, parentTransform, filename) };
    return { markParseResult.name, markParseResult.transform,
        supplyChangeMarkNodeActiveStatus(node, propGuid, filename)};
}

/**
 * Parses the @p node and tries to create a direction mark from it.
 */
DirectionMark processDirectionMarkXmlNode(const pugi::xml_node& node,
    const glm::dmat4& parentTransform, const std::string& filename)
{
    const MarkNodeParseResult markParseResult{ parseMarkXmlNode(node, parentTransform, filename) };
    return {markParseResult.name, markParseResult.transform};
}

/**
 * Parses the @p node and tries to create a supply change warning mark from it.
 */
SupplyChangeWarningMark processWarningMarkXmlNode(const pugi::xml_node& node,
    const glm::dmat4& parentTransform, const std::string& supplySwitchWarningPropGuid,
    const std::string& filename)
{
    const MarkNodeParseResult markParseResult{ parseMarkXmlNode(node, parentTransform, filename) };
    const SupplyChangeWarningPropParams propParams{
        extractSupplyChangeWarningPropParams(node, supplySwitchWarningPropGuid, filename) };

    return SupplyChangeWarningMark{
        markParseResult.name,
        markParseResult.transform,
        propParams.hostMarkName,
        propParams.isAheadMark
    };
}

/**
 * Parses @p parentNode children and tries to create supply change marks from them.
 */
SupplyChangeMarksLoadingResult doLoadSupplyChangeMarks(const pugi::xml_node& parentNode,
    const glm::dmat4& parentTransform, const std::string& supplySwitchPropGuid,
    const std::string& supplySwitchWarningPropGuid, const std::string& filename)
{
    std::vector<SwitcherMark> switcherMarks;
    std::vector<DirectionMark> directionMarks;
    std::vector<SupplyChangeWarningMark> warningMarks;
    std::vector<LayerXmlParseStatus> errors;

    const auto& container = parentNode.children(LayerXmlParseConsts::nodeName());

    std::for_each(container.begin(), container.end(),[&](const pugi::xml_node& node)
    {
        try
        {
            const std::string name{ nodeName(node, filename) };
            if (name.find(SupplyChangeConsts::supplySwitchNodeNameTag()) == 0)
            {
                return switcherMarks.push_back(
                    processSupplyChangeMarkXmlNode(node, parentTransform, supplySwitchPropGuid,
                        filename));
            }
            if (name.find(SupplyChangeConsts::nonelectrifiedZoneDirectionNodeNameTag()) == 0)
            {
                directionMarks.push_back(
                    processDirectionMarkXmlNode(node, parentTransform, filename));
            }
            if (name.find(SupplyChangeConsts::supplySwitchWarningNodeNameTag()) == 0)
            {
                warningMarks.push_back(
                    processWarningMarkXmlNode(node, parentTransform, supplySwitchWarningPropGuid,
                        filename));
            }
        }
        catch (const LayerXmlParseException& except)
        {
            errors.push_back(except.message);
        }
    });

    return {errors, switcherMarks, directionMarks, warningMarks};
}

pugi::xml_document parseXmlFile(const std::filesystem::path& filePath)
{
    pugi::xml_document doc;
    const pugi::xml_parse_result parseResult{ doc.load_file(filePath.c_str()) };
    if (!parseResult) 
    {
        throw LayerXmlParseException({ filePath.string(), LayerXmlParseStatusCode::CannotParseXml });
    }
    return doc;
}
} //anonymous namespace

SupplyChangeMarksLoadingResult loadSupplyChangeMarks(const std::filesystem::path& layerFilePath,
    const std::filesystem::path& propsDirPath, const std::string& xmlRootNodeName,
    const std::shared_ptr<UnigineGuidDatabase>& guidDatabase)
{
    // 1. Open an XML document. We need to keep this document alive until the end of the function,
    // because pugi::xml_node objects refer to it.
    const pugi::xml_document xml{ parseXmlFile(layerFilePath) };

    // 2. Find the parent node for supply change marks.
    const pugi::xml_node marksParentNode{ findMarksRootNode(findNodesRoot(
        xml, layerFilePath.string()), xmlRootNodeName,
        layerFilePath.string()) };

    // 3. Calculate the parent transform.
    const glm::dmat4 parentTransform{ calcTransform(glm::dmat4(1.0),
        findTransformNode(marksParentNode, layerFilePath.string())) };

    // 4. Look for the property GUID in database.
    const std::string supplySwitchPropGuid = findPropertyGuidInDatabse(
        propsDirPath / SupplyChangeConsts::propertyName(), guidDatabase);

    const std::string supplySwitchWarningPropGuid = findPropertyGuidInDatabse(
        propsDirPath / SupplyChangeConsts::supplySwitchWarningPropertyName(), guidDatabase);

    // 5. Parse mark nodes.
    return doLoadSupplyChangeMarks(marksParentNode, parentTransform, supplySwitchPropGuid,
        supplySwitchWarningPropGuid, layerFilePath.string());
}
