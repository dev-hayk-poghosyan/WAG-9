#include "FindWildcardChildren.h"

#include "StringWildcardMatch.h"

#include <cassert>

std::vector<Unigine::NodePtr> findWildcardChildren(
    Unigine::NodePtr const& node, char const* childNamePattern)
{
    assert(node);
    assert(childNamePattern);

    std::vector<Unigine::NodePtr> matchingChildren;

    int const numChildren = node->getNumChildren();
    for (int i = 0; i < numChildren; ++i)
    {
        if (auto childNode = node->getChild(i))
        {
            if (char const* childName = childNode->getName())
            {
                if (stringWildcardMatch(childName, childNamePattern))
                {
                    matchingChildren.push_back(childNode);
                }
            }
        }
    }

    return matchingChildren;
}
