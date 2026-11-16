#pragma once

#include <pugixml.hpp>

pugi::xml_node findUnigineProperty(pugi::xml_node xmlNode, std::string const& propertyGuid);

pugi::xml_node
findUniginePropertyParameter(pugi::xml_node xmlProperty, std::string const& parameterName);
