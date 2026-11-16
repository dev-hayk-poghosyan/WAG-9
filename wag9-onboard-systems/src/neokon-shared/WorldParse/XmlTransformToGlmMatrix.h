#pragma once

#include "glm/glm.hpp"

#include <string>

/**
 * Converts @p transformStr read from the Unigine node xml file into a glm::dmat4.
 */
glm::dmat4 xmlTransformToGlmMatrix(std::string const& transformStr);

/**
 * Converts @p str read from the Unigine node xml file into a glm::ivec2.
 */
glm::ivec2 xmlVecToGlmIVec2(std::string const& str);
