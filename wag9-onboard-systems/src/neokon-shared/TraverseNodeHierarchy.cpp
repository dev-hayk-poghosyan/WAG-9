#include "TraverseNodeHierarchy.h"

#include <UnigineNodes.h>

void
traverseNodeHierarchy(
    Unigine::NodePtr const& node,
    std::function<void(Unigine::NodePtr)> const& sink,
    bool followReferences, Unigine::NodePtr const& excludeHierarchy)
{
    assert(node);
    if (excludeHierarchy && node->getID() == excludeHierarchy->getID())
    {
        return;
    }

    sink(node);

    for (int i = 0; i < node->getNumChildren(); i++)
    {
        traverseNodeHierarchy(node->getChild(i), sink, followReferences, excludeHierarchy);
    }

    if (followReferences)
    {
        if (auto ref = Unigine::checked_ptr_cast<Unigine::NodeReference>(node))
        {
            if (auto referencedNode = ref->getReference())
            {
                traverseNodeHierarchy(referencedNode, sink, followReferences, excludeHierarchy);
            }
        }
    }
}

void traverseNodeHierarchyConditional(
    Unigine::NodePtr const& node,
    std::function<bool(Unigine::NodePtr)> const& sink,
    bool followReferences,
    Unigine::NodePtr const& excludeHierarchy)
{
    assert(node);
    if (excludeHierarchy && node->getID() == excludeHierarchy->getID())
    {
        return;
    }

    if (!sink(node))
    {
        return;
    }

    for (int i = 0; i < node->getNumChildren(); i++)
    {
        traverseNodeHierarchy(node->getChild(i), sink, followReferences, excludeHierarchy);
    }

    if (followReferences)
    {
        if (auto ref = Unigine::checked_ptr_cast<Unigine::NodeReference>(node))
        {
            if (auto referencedNode = ref->getReference())
            {
                traverseNodeHierarchy(referencedNode, sink, followReferences, excludeHierarchy);
            }
        }
    }
}
