#include "LoadNeutralSectionMarks.h"

#include "neokon-shared/GetFileGuidFromMeta.h"
#include "neokon-shared/PlusEnum.h"
#include "neokon-shared/WorldParse/XmlTransformToGlmMatrix.h"

#include <pugixml.hpp>

#include <filesystem>
#include <optional>

namespace
{
const std::string layersDirName("layers");

/**
 * Looks for node with name "NeutralSection" in @p doc.
 */
std::optional<pugi::xml_node>
findNeutralSectionParent(pugi::xml_document const& doc)
{
     for (const auto& child : doc.child("nodes").children("node"))
     {
         if (std::string(child.attribute("name").as_string()) ==
             NeutralSectionConsts::neutralSectionParentName())
         {
             return child;
         }
     }

     return std::nullopt;
 }

NeutralSectionMarkType
neutralSectionMarkTypeFromInt(int value)
{
    assert(value == +NeutralSectionMarkType::Warning || value == +NeutralSectionMarkType::Zone);
    return static_cast<NeutralSectionMarkType>(value);
}

/**
 * Looks for nodes with @p propGuid property among the children of @p xmlNode.
 * @p xmlNode is the group node for all marks of one neutral section.
 */
NeutralSectionMarksLoadingStatus loadMark(pugi::xml_node const& xmlNode,
    glm::dmat4 const& parentTransform, std::string const& propGuid,
    std::string const& layerFileName, std::vector<NeutralSectionMark>& result)
{
    struct NodeParsingResult
    {
        const NeutralSectionMarksLoadingStatus parseStatus;
        const glm::dmat4 transform;
        const std::string name;
        const int id;
    };

    const auto transformAndIdExtractor = [layerFileName](pugi::xml_node const& node,
        glm::dmat4 const& parentTransform)
    {
        auto idAttr = node.attribute("id");
        if (idAttr.empty())
        {
            const auto status = NeutralSectionMarksLoadingStatus(
                NeutralSectionMarksLoadingStatusCode::NodeIdNotFound, layerFileName,
                node.attribute("name").as_string(), {});
            return NodeParsingResult{status, glm::dmat4{}, "", - 1};
        }
        const int id = idAttr.as_int();

        auto transformNode = node.child("transform");
        if (transformNode.empty())
        {
            const auto status = NeutralSectionMarksLoadingStatus(
                NeutralSectionMarksLoadingStatusCode::NodeTransformNotFound, layerFileName,
                node.attribute("name").as_string(), {});
            return NodeParsingResult{status, glm::dmat4{}, "", - 1};
        }

        auto nameAttr = node.attribute("name");
        if (nameAttr.empty())
        {
            const auto status = NeutralSectionMarksLoadingStatus(
                NeutralSectionMarksLoadingStatusCode::NodeNameNotFound, layerFileName, {}, {});
        }

        const glm::dmat4 transform =
            parentTransform * xmlTransformToGlmMatrix(transformNode.text().as_string());

        const auto status = NeutralSectionMarksLoadingStatus(
            NeutralSectionMarksLoadingStatusCode::Success, {}, {}, {});

        return NodeParsingResult{status, transform, nameAttr.as_string(), id};
    };

    const auto [parseStatus, groupTransform, parentName, groupId] =
        transformAndIdExtractor(xmlNode, parentTransform);

    if (parseStatus.statusCode != NeutralSectionMarksLoadingStatusCode::Success)
    {
        return parseStatus;
    }

    for (const auto& child : xmlNode.children("node"))
    {
        const auto [parseStatus, childTransform, childName, childId] =
            transformAndIdExtractor(child, groupTransform);

        if (parseStatus.statusCode != NeutralSectionMarksLoadingStatusCode::Success)
        {
             return parseStatus;
        }

        const auto props(child.child("properties"));
        if (!props)
        {
             continue;
        }

        for (const auto& prop : props.children("property"))
        {
             if (prop.attribute("parent").as_string() != propGuid)
             {
                 continue;
             }

             NeutralSectionMarkType type = NeutralSectionMarkType::Zone;
             bool isActive = true;
             for (const auto& param : prop.children("parameter"))
             {
                 if (std::string(param.attribute("name").as_string()) ==
                     NeutralSectionConsts::neutralSectionPropParamName())
                 {
                     type = neutralSectionMarkTypeFromInt(param.text().as_int());
                 }
                 if (std::string(param.attribute("name").as_string()) ==
                     NeutralSectionConsts::neutralSectionPropActiveParamName())
                 {
                     isActive = param.text().as_bool();
                 }
             }

             result.push_back(NeutralSectionMark(childId, parentName, type,
                 std::string(child.attribute("name").as_string()), childTransform, isActive));
        }
    }

    return NeutralSectionMarksLoadingStatus(NeutralSectionMarksLoadingStatusCode::Success, {}, {},
        {});
}

} // anonymous namespace.

const char*
NeutralSectionConsts::neutralSectionParentName()
{
    return "NeutralSection";
}

std::string
NeutralSectionConsts::propertiesDirName()
{
    return "properties";
}

std::string
NeutralSectionConsts::neutralSectionPropFileName()
{
    return "NeutralSection.prop";
}

std::string
NeutralSectionConsts::neutralSectionPropParamName()
{
    return "MarkType";
}

std::string
NeutralSectionConsts::neutralSectionPropActiveParamName()
{
    return "IsActive";
}

NeutralSectionMarksLoadingStatus
loadNeutralSectionMarks(std::string const& worldDirName, std::string const& terrainEditorDataPath,
    std::string const& layerFileName, std::vector<NeutralSectionMark>& result)
{
    const std::filesystem::path layerPath =
        std::filesystem::path(worldDirName) / layersDirName / layerFileName;

    if (!std::filesystem::exists(layerPath))
    {
        NeutralSectionMarksLoadingStatus(NeutralSectionMarksLoadingStatusCode::FileNotFound,
            layerPath.string(), {}, {});
    }

    pugi::xml_document doc;
    const pugi::xml_parse_result parseResult = doc.load_file(layerPath.c_str());

    if (!parseResult)
    {
        return NeutralSectionMarksLoadingStatus(
            NeutralSectionMarksLoadingStatusCode::CannotParseXml, layerPath.string(), {},
            std::string(parseResult.description()));
    }

    auto neutralSectionParent = findNeutralSectionParent(doc);
    if (!neutralSectionParent)
    {
        return NeutralSectionMarksLoadingStatus(
            NeutralSectionMarksLoadingStatusCode::CannotFindRootInXml, layerPath.string(),
            NeutralSectionConsts::neutralSectionParentName(), {});
    }

    const pugi::xml_node parentTransformNode = neutralSectionParent->child("transform");
    if (!parentTransformNode)
    {
        return NeutralSectionMarksLoadingStatus(
            NeutralSectionMarksLoadingStatusCode::NodeTransformNotFound, layerPath.string(),
            NeutralSectionConsts::neutralSectionParentName(), {});
    }
    const glm::dmat4 parentTransform(
        xmlTransformToGlmMatrix(parentTransformNode.text().as_string()));

    const std::string propGuid(getFileGuidFromMeta(terrainEditorDataPath,
        std::filesystem::path(NeutralSectionConsts::propertiesDirName()) /
            NeutralSectionConsts::neutralSectionPropFileName()));
    if (propGuid.empty())
    {
        return NeutralSectionMarksLoadingStatus(
            NeutralSectionMarksLoadingStatusCode::PropertyGuidNotFound,
            NeutralSectionConsts::neutralSectionPropFileName() + ".meta", {}, {});
    }

    for (const auto& child : neutralSectionParent->children("node"))
    {
        const NeutralSectionMarksLoadingStatus loadStatus(loadMark(child, parentTransform, propGuid,
            layerPath.string(), result));
        if (loadStatus.statusCode != NeutralSectionMarksLoadingStatusCode::Success)
        {
             return loadStatus;
        }
    }

    return NeutralSectionMarksLoadingStatus(NeutralSectionMarksLoadingStatusCode::Success, {}, {},
        {});
}

