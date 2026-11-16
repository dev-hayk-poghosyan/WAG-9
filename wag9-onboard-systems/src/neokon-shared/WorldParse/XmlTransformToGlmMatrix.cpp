#include "XmlTransformToGlmMatrix.h"

glm::dmat4
xmlTransformToGlmMatrix(std::string const& transformStr)
{
    glm::dmat4 mat;

    char const* text = transformStr.c_str();
    char* next;

    for (int col = 0; col < 4; ++col)
    {
        for (int row = 0; row < 4; ++row)
        {
            mat[col][row] = strtod(text, &next);
            text = next;
        }
    }

    return mat;
}

glm::ivec2 xmlVecToGlmIVec2(std::string const& str)
{
    glm::ivec2 vec;
    char const* text = str.c_str();
    char* next;

    for (int element = 0; element < 2; ++element)
    {
        vec[element] = strtol(text, &next, 10);
        text = next;
    }

    return vec;
}
