#include "XmlProperties.h"

pugi::xml_node
findUnigineProperty(pugi::xml_node xmlNode, std::string const& propertyGuid)
{
    auto xmlProperties = xmlNode.child("properties");
    if (!xmlProperties)
    {
        return {};
    }

    for (auto xmlProperty : xmlProperties.children("property"))
    {
        if (xmlProperty.attribute("parent").as_string() == propertyGuid)
        {
            return xmlProperty;
        }
    }
    return {};
}

pugi::xml_node
findUniginePropertyParameter(pugi::xml_node xmlProperty, std::string const& parameterName)
{
    for (auto xmlParameter : xmlProperty.children("parameter"))
    {
        if (xmlParameter.attribute("name").as_string() == parameterName)
        {
            return xmlParameter;
        }
    }

    return {};
}
