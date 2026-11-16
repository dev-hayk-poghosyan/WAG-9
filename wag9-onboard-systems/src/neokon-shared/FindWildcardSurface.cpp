#include "FindWildcardSurface.h"

#include "StringWildcardMatch.h"

std::optional<int> findWildcardSurface(
    Unigine::Object const& obj, char const* surfaceNamePattern, unsigned int startFrom)
{
    unsigned int const numSurfaces = obj.getNumSurfaces();
    for (unsigned int i = startFrom; i < numSurfaces; ++i)
    {
        char const* surfaceName = obj.getSurfaceName(i);
        
        if (stringWildcardMatch(surfaceName, surfaceNamePattern))
        {
            return i;
        }
    }

    return std::nullopt;
}
