#include "ResolveNodePath.h"

#include "FindWildcardChild.h"

#include <boost/format.hpp>
#include <boost/tokenizer.hpp>
#include <UnigineNodes.h>

#include <cassert>
#include <stdexcept>
#include <string>

namespace
{

void throwException(char const* exceptionStringPrefix, std::string const& exceptionString)
{
    if (exceptionStringPrefix)
    {
        throw std::logic_error(exceptionStringPrefix + exceptionString);
    }
    else
    {
        throw std::logic_error(exceptionString);
    }
}

Unigine::NodePtr processPathElementOrThrow(
    Unigine::NodePtr const& currentNode, char const* path,
    char const* elBegin, char const* elEnd, bool hasWildcards,
    char const* exceptionStringPrefix)
{
    Unigine::NodePtr newNode;

    if (hasWildcards)
    {
        newNode = findWildcardChild(currentNode, std::string(elBegin, elEnd).c_str());
        if (!newNode)
        {
            throwException(exceptionStringPrefix, str(boost::format(
                "Wildcard match failed at the end of \"%s\"") % std::string(path, elEnd)));
        }
    }
    else if (*elBegin == '@' && elBegin + 1 == elEnd)
    {
        auto nodeRef = Unigine::checked_ptr_cast<Unigine::NodeReference>(currentNode);
        if (!nodeRef || !(newNode = nodeRef->getReference()))
        {
            throwException(exceptionStringPrefix, str(boost::format(
                "Dereferencing failed at the end of \"%s\"") % std::string(path, elEnd)));
        }
    }
    else
    {
        newNode = currentNode->findNode(std::string(elBegin, elEnd).c_str(), /*recursive=*/0);
        if (!newNode)
        {
            throwException(exceptionStringPrefix, str(boost::format(
                "Child not found at the end of \"%s\"") % std::string(path, elEnd)));
        }
    }

    return newNode;
}

Unigine::NodePtr tryProcessPathElement(
    Unigine::NodePtr const& currentNode, char const* path,
    char const* elBegin, char const* elEnd, bool hasWildcards)
{
    Unigine::NodePtr newNode;

    if (hasWildcards)
    {
        newNode = findWildcardChild(currentNode, std::string(elBegin, elEnd).c_str());
    }
    else if (*elBegin == '@' && elBegin + 1 == elEnd)
    {
        if (auto nodeRef = Unigine::checked_ptr_cast<Unigine::NodeReference>(currentNode))
        {
            newNode = nodeRef->getReference();
        }
    }
    else
    {
        newNode = currentNode->findNode(std::string(elBegin, elEnd).c_str(), /*recursive=*/0);
    }

    return newNode;
}

} // anon namespace


Unigine::NodePtr resolveNodePathOrThrow(
    Unigine::NodePtr const& rootNode, char const* path,
    char const* exceptionStringPrefix)
{
    assert(rootNode);
    assert(path);

    Unigine::NodePtr currentNode = rootNode;

    for (char const* p = path;;)
    {
        char const* elBegin = p;

        bool hasWildcards = false;
        while (*p && *p != '/')
        {
            if (*p == '*')
            {
                hasWildcards = true;
            }
            ++p;
        }
        
        char const* elEnd = p;

        if (elEnd != elBegin)
        {
            currentNode = processPathElementOrThrow(
                currentNode, path, elBegin, elEnd, hasWildcards, exceptionStringPrefix);
        }

        if (*p)
        {
            assert(*p == '/');
            ++p;
        }
        else
        {
            break;
        }
    }

    return currentNode;
}

Unigine::NodePtr tryResolveNodePath(
    Unigine::NodePtr const& rootNode, char const* path)
{
    assert(rootNode);
    assert(path);

    Unigine::NodePtr currentNode = rootNode;

    for (char const* p = path;;)
    {
        char const* elBegin = p;

        bool hasWildcards = false;
        while (*p && *p != '/')
        {
            if (*p == '*')
            {
                hasWildcards = true;
            }
            ++p;
        }

        char const* elEnd = p;

        if (elEnd != elBegin)
        {
            currentNode = tryProcessPathElement(
                currentNode, path, elBegin, elEnd, hasWildcards);
            if (!currentNode)
            {
                break;
            }
        }

        if (*p)
        {
            assert(*p == '/');
            ++p;
        }
        else
        {
            break;
        }
    }

    return currentNode;
}

Unigine::NodePtr resolveNodePathsOrThrow(Unigine::NodePtr const& rootNode,
    std::vector<std::string> const& paths, char const* exceptionStringPrefix)
{
    assert(!paths.empty());

    Unigine::NodePtr node;
    for (auto const& path : paths)
    {
        if (!node)
        {
            node = tryResolveNodePath(rootNode, path.c_str());
        }
        else
        {
            return node;
        }
    }

    if (node)
    {
        return node;
    }

    throwException(exceptionStringPrefix, str(boost::format(
        "Couldn't resolve node path for node \"%1%\" and first path \"%2%\"") %
        rootNode->getName() % paths[0]));

    return node;
}

Unigine::NodePtr tryResolveNodePaths(Unigine::NodePtr const& rootNode,
    std::vector<std::string> const& paths)
{
    Unigine::NodePtr node;
    for (auto const& path : paths)
    {
        if (!node)
        {
            node = tryResolveNodePath(rootNode, path.c_str());
        }
        else
        {
            return node;
        }
    }

    return node;
}
