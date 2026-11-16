#include <UnigineMathLib.h>


/**
 * This function calculates affine transform matrix which moves points 'from0' to 'to0' and 'from1' to 'to1'.
 * The typical usage is snapping (scale+rotate+transform) some object with another object's points.
 */
Unigine::Math::Mat4 transformFromTwoPointPairs(const Unigine::Math::Vec3& from0,
    const Unigine::Math::Vec3& to0, const Unigine::Math::Vec3& from1,
    const Unigine::Math::Vec3& to1);
