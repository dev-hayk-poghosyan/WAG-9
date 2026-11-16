#include "CalcOffsetedBezierSegment.h"
#include "GlmUnigineInterop.h"
#include "TransformFromPositionAndAxis.h"

using namespace Unigine;

Math::Mat4
calcSplinePointMatrix(BezierSegment const& segment, double param,
    TransformOrthogonalization orthogonalization)
{
    glm::dvec3 glmPosition = segment.eval(param);
    glm::dvec3 glmTangent = segment.unitTangentAt(param);

    Math::Vec3 position = fromGlm(glmPosition);
    Math::Vec3 tangent = fromGlm(glmTangent);

    if (orthogonalization == TransformOrthogonalization::On)
    {
        return transformFromTranslateAndDirectionDirXUpZ(position, tangent);
    }
    return transformFromPositionAndTangent(position, tangent);
};

Math::Mat4
calcBezierSegmentDistanceMatrix(const BezierSegment& segment, double distance,
    TransformOrthogonalization orthogonalization)
{
    const double t = segment.tAtDistance(distance);

    return calcSplinePointMatrix(segment, t, orthogonalization);
}

BezierSegment calcOffsetedBezierSegment(BezierSegment const& srcSegment, const glm::dvec3& offset)
{
    auto unigineOffset = fromGlm(offset);

    Math::Vec3 p0 = calcSplinePointMatrix(srcSegment, 0.) * unigineOffset;
    Math::Vec3 p3 = calcSplinePointMatrix(srcSegment, 1.) * unigineOffset;

    Math::Vec3 y1 = calcSplinePointMatrix(srcSegment, 0.3333) * unigineOffset;
    Math::Vec3 y2 = calcSplinePointMatrix(srcSegment, 0.6667) * unigineOffset;
    Math::Mat4 y(p0, y1, y2, p3);

    Math::Vec3 p1(y * cubicBezierControlP1);
    Math::Vec3 p2(y * cubicBezierControlP2);

    return BezierSegment(toGlm(p0), toGlm(p1), toGlm(p2), toGlm(p3));
}
