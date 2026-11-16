#include "XmlIO.h"

#include "neokon-shared/WorldParse/XmlTransformToGlmMatrix.h"

#include <boost/algorithm/string/trim.hpp>

#include <stdexcept>
#include <sstream>
#include <iomanip>

pugi::xml_document
XmlIO::loadXML(QString const& filePath, unsigned int parseOptions)
{
    pugi::xml_document doc;
    auto const result = doc.load_file(filePath.toStdWString().c_str(), parseOptions);

    if (!result)
    {
        throw std::runtime_error(
            tr("Ошибка загрузки документа \"%1\":\n%2").arg(
                filePath, QString::fromUtf8(result.description())).toStdString());
    }

    return doc;
}

std::unordered_map<std::string, glm::dmat4>
XmlIO::loadTopLevelNodeTransforms(pugi::xml_node const& worldXml)
{
    std::unordered_map<std::string, glm::dmat4> map;

    for (auto const& node : worldXml.child("world").child("editor").children("node"))
    {
        std::string const name      = node.attribute("name").as_string();
        std::string const transform = node.child("transform").text().as_string();

        if (!name.empty() && !transform.empty())
        {
            map.try_emplace(name, xmlTransformToGlmMatrix(transform));
        }
    }

    return map;
}

std::string
XmlIO::toUnigineStringTransform(glm::dmat4 const& mat)
{
    std::ostringstream str;
    str << std::fixed;

    for (int col = 0; col < glm::dmat4::length(); ++col)
    {
        for (int row = 0; row < glm::dmat4::length(); ++row)
        {
            str << mat[col][row] << " ";
        }
    }
    return boost::algorithm::trim_copy(str.str());
}

void
XmlIO::processNodesWithTransforms(pugi::xml_node const& rootNode,
    glm::dmat4 const & worldFromRootParent,
    std::function<void(pugi::xml_node const& tree,
        glm::dmat4 const&worldFromNode)> const & sink)
{
    /**
    * post-order traversal
    */
    std::function<void(pugi::xml_node const&, glm::dmat4 const&)>
        processTree;

    processTree = [&processTree, &sink]
    (pugi::xml_node const& node, glm::dmat4 const& worldFromParent)
    {
        sink(node, worldFromParent);
        glm::dmat4 const parentFromLocal =
            xmlTransformToGlmMatrix(node.child("transform").text().as_string());

        glm::dmat4 const worldFromLocal = worldFromParent * parentFromLocal;
        for (const auto &kv : node)
        {
            if (!kv.attribute("enabled").as_bool(true))
                continue;

            processTree(kv, worldFromLocal);
        }
    };

    processTree(rootNode, worldFromRootParent);
}

bool
XmlIO::nodeHasUnigineProperty(pugi::xml_node const& node, std::string const& propGuid)
{
    auto const checkForProperty = [&propGuid](pugi::xml_node const& propsNode)
    {
        for (auto const& property : propsNode.children("property"))
        {
            std::string const parent = property.attribute("parent").as_string();

            if (parent == propGuid)
                return true;
        }
        return false;
    };

    for (const auto &properties : node.children("properties"))
    {
        if (checkForProperty(properties))
        {
            return true;
        }
    }
    return false;
}

pugi::xml_node
XmlIO::getChildOrCreate(pugi::xml_node node, const char* name)
{
    pugi::xml_node child = node.child(name);
    if (!child)
        child = node.append_child(name);

    return child;
}

pugi::xml_attribute
XmlIO::getAttrOrCreate(pugi::xml_node node, const char* attrName)
{
    pugi::xml_attribute attr = node.attribute(attrName);
    if (!attr)
        attr = node.append_attribute(attrName);

    return attr;
}

pugi::xml_node
XmlIO::getChildByNameAttrOrCreate(pugi::xml_node node,
    const char* nodeName, const char* nameAttrValue)
{
    pugi::xml_node resultNode;
    for (pugi::xml_node child : node.children(nodeName))
    {
        if (nameAttrValue == std::string(child.attribute("name").as_string()))
        {
            resultNode = child;
            break;
        }
    }

    if (!resultNode)
        resultNode = node.append_child(nodeName);

    getAttrOrCreate(resultNode, "name").set_value(nameAttrValue);

    return resultNode;
}

