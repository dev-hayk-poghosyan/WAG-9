#include "StringWildcardMatch.h"

#include <cassert>

bool stringWildcardMatch(const char* string, const char* wild)
{
    // This function was borrowed from here:
    // https://www.codeproject.com/Articles/1088/Wildcard-string-compare-globbing
    // and modified to remove support for the '?' special character.
    // The original author is Jack Handy <jakkhandy@hotmail.com>

    assert(string);
    assert(wild);

    const char *cp = nullptr;
    const char *mp = nullptr;

    while ((*string) && (*wild != '*'))
    {
        if (*wild != *string)
        {
            return false;
        }

        wild++;
        string++;
    }

    while (*string)
    {
        if (*wild == '*')
        {
            if (!*++wild)
            {
                return true;
            }

            mp = wild;
            cp = string + 1;
        }
        else if (*wild == *string)
        {
            wild++;
            string++;
        }
        else
        {
            wild = mp;
            string = cp++;
        }
    }

    while (*wild == '*')
    {
        wild++;
    }

    return *wild == '\0';
}
