#include "BezierSegmentHelpers.h"

#include "neokon-shared/GlmUnigineInterop.h"

BezierSegment
bezierSegmentByPositionsAndTangents(Unigine::Math::Vec3 const& startPointPosition,
    Unigine::Math::Vec3 const& endPointPosition, Unigine::Math::Vec3 const& startPointTangent,
    Unigine::Math::Vec3 const& endPointTangent)
{
    Unigine::Math::Vec3 p0 = startPointPosition;
    Unigine::Math::Vec3 p3 = endPointPosition;
    Unigine::Math::Vec3 p1 = p0 + startPointTangent;
    Unigine::Math::Vec3 p2 = p3 + endPointTangent;
    return BezierSegment(toGlm(p0), toGlm(p1), toGlm(p2), toGlm(p3));
}

std::optional<BezierSegment>
bezierSegmentByThreePoints(Unigine::Math::Vec3 const& startPoint,
    Unigine::Math::Vec3 const& middlePoint, Unigine::Math::Vec3 const& endPoint)
{
    glm::dvec3 startPos = toGlm(startPoint);
    glm::dvec3 endPos = toGlm(endPoint);
    glm::dvec3 midPos = toGlm(middlePoint);

    glm::dvec3 tangentPoint = midPos * 2. - (endPos + startPos) * 0.5;

    double coef = 2. / 3;
    glm::dvec3 p1 = startPos + (tangentPoint - startPos) * coef;
    glm::dvec3 p2 = endPos + (tangentPoint - endPos) * coef;

    return BezierSegment(startPos, p1, p2, endPos);
}
