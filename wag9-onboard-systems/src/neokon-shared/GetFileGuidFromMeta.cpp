#include "GetFileGuidFromMeta.h"

#include <boost/format.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <string>

namespace pt = boost::property_tree;
namespace ptxml = pt::xml_parser;

std::string getFileGuidFromMeta(
    std::filesystem::path const& terrainEditorDataAbsDirPath,
    std::filesystem::path const& propertyRelFilePath)
{
    std::string guid = "";
    std::filesystem::path fullPropertyFilePath(terrainEditorDataAbsDirPath / propertyRelFilePath);
    fullPropertyFilePath += ".meta";
    std::ifstream strm(fullPropertyFilePath);
    pt::ptree tree;
    ptxml::read_xml(strm, tree);
    tree = tree.get_child("asset");
    guid = tree.get<std::string>("guid");
    if (guid.size() != 40)
    {
        throw std::logic_error(boost::str(boost::format("Can`t find \"guid\" in the \"%1%\"")
            % fullPropertyFilePath.u8string()));
    }
    return guid;
}
