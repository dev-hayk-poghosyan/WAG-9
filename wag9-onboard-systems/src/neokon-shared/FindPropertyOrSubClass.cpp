#include "FindPropertyOrSubClass.h"

Unigine::PropertyPtr findPropertyOrSubClass(Unigine::NodePtr const& node, char const* name)
{
    int numProperties = node->getNumProperties();
    for (int idx = 0; idx < numProperties; ++idx)
    {
        auto const property = node->getProperty(idx);

        // isParent() returns true for the property itself or any of its
        // direct or indirect descendants.
        if (property->isParent(name))
        {
            return property;
        }
    }
    return nullptr;
}
