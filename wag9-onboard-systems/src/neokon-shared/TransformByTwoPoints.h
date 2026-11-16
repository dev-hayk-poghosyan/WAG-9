#pragma once

#include <UnigineMathLib.h>

/**
 * @return transformation matrix based on vector from @point0 to @point1
 * differs from getTransformFromTangentAndPosition() (MathUtils.h) by preserving heights.
 */
Unigine::Math::Mat4 transformByTwoPoints(Unigine::Math::Vec3 const& point0,
    Unigine::Math::Vec3 const& point1);

/**
 * @return transformation matrix based on vector from @point0 to @point1 and vector from @point0 to @point2
 * differs from getTransformFromTangentAndPosition() (MathUtils.h) by preserving heights.
 */
Unigine::Math::Mat4 transformByThreePoints(Unigine::Math::Vec3 const& point0,
    Unigine::Math::Vec3 const& point1, Unigine::Math::Vec3 const& point2);
