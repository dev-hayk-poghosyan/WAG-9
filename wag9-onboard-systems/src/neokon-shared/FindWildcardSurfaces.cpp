#include "FindWildcardSurfaces.h"

#include "StringWildcardMatch.h"

std::vector<int> findWildcardSurfaces(Unigine::Object const& obj,
    char const* surfaceNamePattern, std::vector<std::string>* surfaceNames)
{
    std::vector<int> surfaceIds;

    int const numSurfaces = obj.getNumSurfaces();
    for (int i = 0; i < numSurfaces; ++i)
    {
        char const* surfaceName = obj.getSurfaceName(i);
        
        if (stringWildcardMatch(surfaceName, surfaceNamePattern))
        {
            surfaceIds.push_back(i);
            if (surfaceNames)
            {
                surfaceNames->push_back(surfaceName);
            }
        }
    }

    return surfaceIds;
}
