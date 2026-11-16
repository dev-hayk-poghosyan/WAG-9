#include "MetaUnigine.h"
#include "MetaUnigineObjects.h"
#include "MetaUnigineWorldSplineGraph.h"

#include <UnigineString.h>
using namespace Unigine;

namespace MetaUnigine
{

bool saveNode(const char* nodeFilePath, const NodePtr& node, int binary)
{
    auto xmlNode = Xml::create("nodes");
    xmlNode->setArg("version", "2.13.0.0");

    node->saveWorld(xmlNode);

    return xmlNode->save(nodeFilePath, binary == 1);
}

bool saveNodes(const char* nodeFilePath, const std::vector<NodePtr>& nodes, int binary)
{
    auto xmlNode = Xml::create("nodes");
    xmlNode->setArg("version", "2.13.0.0");

    for (const auto& node : nodes)
    {
        node->saveWorld(xmlNode);
    }
    return xmlNode->save(nodeFilePath, binary == 1);
}

NodePtr loadNode(const XmlPtr& xml)
{
    NodePtr result;

    std::string type(xml->getArg("type"));
    if (type == "ObjectMeshStatic")
    {
        result = std::make_shared<ObjectMeshStatic>();
    }
    else if (type == "ObjectMeshSkinned")
    {
        result = std::make_shared<ObjectMeshSkinned>();
    }
    else if (type == "WorldTransformBone")
    {
        result = std::make_shared<WorldTransformBone>();
    }
    else if (type == "NodeDummy")
    {
        result = std::make_shared<NodeDummy>();
    }
    else if (type == "NodeReference")
    {
        result = std::make_shared<NodeReference>();
    }
    else
    {
        result = std::make_shared<NodeNotSupported>();
    }

    if (!result)
    {
        Log::error("MetaUnigine::loadNode(): \"%s\" type not supported\n", type.data());
        return {};
    }

    if (!result->loadXml(xml))
    {
        return {};
    }
    
    for (int xmlIndex = 0; xmlIndex < xml->getNumChildren(); ++xmlIndex)
    {
        const auto& xmlChild = xml->getChild(xmlIndex);
        if (!strcmp(xmlChild->getName(), "node"))
        {
            auto childNode = loadNode(xmlChild);
            if (!childNode)
            {
                return {};
            }
            childNode->setParent(result);
        }
    }

    return result;
}

NodePtr loadNode(const char* nodeFilePath, bool checkValid)
{
    if (strlen(nodeFilePath) == 0)
    {
        return {};
    }
    auto xmlNodes = Xml::create();
    if (!xmlNodes->load(nodeFilePath) || !xmlNodes->getNumChildren())
    {
        return {};
    }
    auto res = loadNode(xmlNodes->getChild(0));
    if (checkValid && res && !res->isValid())
    {
        return {};
    }
    return res;
}

std::vector<NodePtr> loadNodes(const char* nodeFilePath, bool checkValid)
{
    auto xmlNodes = Xml::create();
    if (!xmlNodes->load(nodeFilePath) || !xmlNodes->getNumChildren())
    {
        return {};
    }
    std::vector<NodePtr> result;
    for (int childIdx = 0; childIdx < xmlNodes->getNumChildren(); ++childIdx)
    {
        auto node = loadNode(xmlNodes->getChild(0));
        if (node && (!checkValid || node->isValid()))
        {
            result.emplace_back(node);
        }
    }
    return result;
}

}//namespace MetaUnigine
