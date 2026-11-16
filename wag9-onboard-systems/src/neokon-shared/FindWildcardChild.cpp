#include "FindWildcardChild.h"

#include "StringWildcardMatch.h"

#include <cassert>

Unigine::NodePtr findWildcardChild(
    Unigine::NodePtr const& node, char const* childNamePattern)
{
    assert(node);
    assert(childNamePattern);

    int const numChildren = node->getNumChildren();
    for (int i = 0; i < numChildren; ++i)
    {
        if (auto childNode = node->getChild(i))
        {
            if (char const* childName = childNode->getName())
            {
                if (stringWildcardMatch(childName, childNamePattern))
                {
                    return childNode;
                }
            }
        }
    }

    return Unigine::NodePtr();
}
