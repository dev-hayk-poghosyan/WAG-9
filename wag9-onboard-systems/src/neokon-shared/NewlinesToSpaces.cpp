#include "NewlinesToSpaces.h"

std::string newlinesToSpaces(std::string_view str)
{
    std::string result;
    result.reserve(str.size());

    for (char ch : str)
    {
        switch (ch)
        {
        case '\r':
            break;
        case '\n':
            result += ' ';
            break;
        default:
            result += ch;
            break;
        }
    }

    return result;
}
