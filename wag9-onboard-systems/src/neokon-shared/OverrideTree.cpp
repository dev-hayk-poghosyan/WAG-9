#include "OverrideTree.h"

#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/find.hpp>

namespace pt = boost::property_tree;
namespace
{
void
overrideTreeImpl(
    pt::ptree& baseTree,
    pt::ptree const& overrideTree,
    pt::ptree const& overrideTreeIterator,
    std::string const& xmlPath)
{
    static const std::string delimiter{ "." };

    for (auto& [name, node] : overrideTreeIterator)
    {
        std::string const currentPath = xmlPath.empty()
            ? name
            : xmlPath + delimiter + name;

        bool const isXmlAttributeNode =
            boost::contains(currentPath, boost::property_tree::xml_parser::xmlattr<std::string>());

        if (node.empty() && !isXmlAttributeNode)
        {
            baseTree.put(currentPath, overrideTree.get<std::string>(currentPath, std::string()));
        }
        else if (isXmlAttributeNode)
        {
            // Source tree contains a node with attribute.
            // We can't identify a node at the destination so just replace all sibling elements.
            if (auto it = boost::find_last(xmlPath, delimiter); it)
            {
                std::string const parentNodePath(xmlPath.begin(), std::prev(it.end()));
                std::string const lastNodeName(it.end(), xmlPath.end());

                // To have called the non-throw variant of ptree::get_child.
                pt::ptree dummy;

                baseTree.get_child(parentNodePath, dummy).erase(lastNodeName);
            }
            else
            {
                baseTree.erase(xmlPath);
            }

            baseTree.add_child(xmlPath, overrideTree.get_child(xmlPath));
        }
        else
        {
            overrideTreeImpl(baseTree, overrideTree, node, currentPath);
        }
    }
}
}

void
overrideTree(boost::property_tree::ptree& baseTree, boost::property_tree::ptree const& overrideTree)
{
    overrideTreeImpl(baseTree, overrideTree, overrideTree, std::string());
}
