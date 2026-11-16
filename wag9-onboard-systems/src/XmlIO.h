#pragma once

#include <QCoreApplication>
#include <QString>

#include <pugixml.hpp>
#include <glm/glm.hpp>

#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

class XmlIO
{
    Q_DECLARE_TR_FUNCTIONS(XmlIO)

public:
    /**
     * Loads an XML file into a pugi::xml_document.
     *
     * @throw std::exception If anything goes wrong.
     */
    static pugi::xml_document loadXML(QString const& filePath,
        unsigned int parseOptions = pugi::parse_default);

    /**
     * Loads the transformation matrices for each top-level node in the world.
     *
     * Returns a map indexed my node names.
     *
     * @throw std::exception If anything goes wrong.
     */
    static std::unordered_map<std::string, glm::dmat4> loadTopLevelNodeTransforms(
        pugi::xml_node const& worldXml);

    /**
     * Converts a 4x4 transformation matrix to string representation, as
     * encountered in Unigine *.world and *.node files.
     *
     * @throw std::exception If anything goes wrong.
     */
    static std::string toUnigineStringTransform(glm::dmat4 const& mat);

    /**
     * Traverses the hierarchy of <node> elements, calculating the node-to-world matrix
     * for each of them.
     *
     * The <node> elements along with their node-to-world matrices are reported via a callback.
     *
     * @param rootNode The root node of our hierarchy. This has to be a <node> element.
     * @param worldFromRootParent The matrix that maps the position of @p rootNode to world coordinates.
     * @param callback Gets called for each <node> element encountered, including @p rootNode.
     *
     * @throw std::exception If anything goes wrong.
     */
    static void processNodesWithTransforms(pugi::xml_node const& rootNode,
        glm::dmat4 const& worldFromRootParent,
        std::function<void(pugi::xml_node const& tree,
            glm::dmat4 const& worldFromNode)> const& sink);

    /**
     * Checks whether the given <node> element has a Unigine property with the specified GUID.
     *
     * @throw std::exception If anything goes wrong.
     */
    static bool nodeHasUnigineProperty(pugi::xml_node const& node,
        std::string const& propGuid);

    /**
     * Returns a first existing child of @p node with @p name or created and added to @p node.
     */
    static pugi::xml_node getChildOrCreate(pugi::xml_node node, const char* name);

    /**
     * Returns an attribute with name @p attrName of @p node.
     * Appends such attribute if it doesn't exist and returns it.
     */
    static pugi::xml_attribute getAttrOrCreate(pugi::xml_node node, const char* attrName);

    /**
     * Returns a child of @p node with attribute 'name' with value @p nameAttrValue.
     * Appends such child if it doesn't exist and returns it.
     */
    static pugi::xml_node getChildByNameAttrOrCreate(pugi::xml_node node,
        const char* nodeName, const char* nameAttrValue);

};
