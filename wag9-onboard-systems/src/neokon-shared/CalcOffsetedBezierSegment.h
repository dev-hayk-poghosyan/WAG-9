#pragma once

#include "BezierSegment.h"
#include <UnigineMathLib.h>

/**
 * Cubic bezier control point recovery from positions at t=1/3 and t=2/3. Taken from here:
 * https://web.archive.org/web/20131225210855/http://people.sc.fsu.edu/~jburkardt/html/bezier_interpolation.html
 */
const Unigine::Math::Vec4 cubicBezierControlP1(-5. / 6., 18. / 6., -9. / 6., 2. / 6.);
const Unigine::Math::Vec4 cubicBezierControlP2(2. / 6., -9 / 6., 18. / 6., -5. / 6.);

enum class TransformOrthogonalization
{
    Off,
    On // Think Gram-Schmidt orthogonalization.
};

/**
 * Calc transform matrix at 'param' of 'segment'
 */
Unigine::Math::Mat4 calcSplinePointMatrix(BezierSegment const& segment, double param,
    TransformOrthogonalization orthogonalization = TransformOrthogonalization::On);

Unigine::Math::Mat4 calcBezierSegmentDistanceMatrix(const BezierSegment& segment, double distance,
    TransformOrthogonalization orthogonalization = TransformOrthogonalization::On);

/**
 * Calc 4 offseted points at 0.0, 0.3333, 0.6667, 1.0 form srcSegment and reconsruct qubic Bezier from them
 */
BezierSegment calcOffsetedBezierSegment(BezierSegment const& srcSegment, const glm::dvec3& offset);
