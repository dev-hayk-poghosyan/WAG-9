#include "FindProperty.h"

Unigine::PropertyPtr findProperty(Unigine::NodePtr const& node, char const* name)
{
    int propertyIdx = node->findProperty(name);
    if (propertyIdx >= 0)
    {
        auto const property = node->getProperty(propertyIdx);
        return property;
    }

    return nullptr;
}
