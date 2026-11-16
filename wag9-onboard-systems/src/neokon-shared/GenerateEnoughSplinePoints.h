#pragma once

#include <functional>
#include <UnigineVector.h>
#include <UnigineMathLib.h>

/**
 * Generate enough points to represent curveness of line,
 * 'calcPoint' calc coords from normalized 0 - 1 value,
 * 'points' store output positions,
 * 'tParams' store output tParams,
 * 'ta' start normalized value to use,
 * 'tb' end normalized value to use
 */
void generateEnoughSplinePoints(const std::function<Unigine::Math::Vec3(double)>& calcPoint,
    Unigine::Vector<Unigine::Math::Vec3>& points, std::vector<float>& tParams, float ta, float tb);

/**
 * Generate enough points to represent curveness of line, calc tangentialOffsets for all points,
 * 'calcPoint' calc coords from normalized 0 - 1 value,
 * 'points' store output positions,
 * 'tParams' store output tParams,
 * 'tangentialOffsetsSquared' store output squared tangantial offsets,
 * 'ta' start normalized value to use,
 * 'tb' end normalized value to use
 */
void generateEnoughSplinePoints(const std::function<Unigine::Math::Vec3(double)>& calcPoint,
    Unigine::Vector<Unigine::Math::Vec3>& points, std::vector<float>& tParams,
    std::vector<double>& tangentialOffsetsSquared, float ta, float tb);
