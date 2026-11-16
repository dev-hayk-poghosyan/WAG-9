#include "BezierSplineStraighten.h"

#include <glm/glm.hpp>

namespace
{
double
length2(glm::dvec3 const& v)
{
    return dot(v, v);
}
} // namespace

BezierSpline bezierSplineStraightenHeavy(BezierSpline const& srcSpline)
{
    auto startPoint = srcSpline.pointAtDistance(std::numeric_limits<double>::epsilon());
    if (!startPoint)
    {
        return srcSpline;
    }

    BezierSpline dstSpline;
    glm::dvec3 prevPoint = *startPoint;
    std::vector<BezierSegment> segments = srcSpline.buildSegmentsVector();
    for (BezierSegment const& segment : segments)
    {
        double distanceToP0 = length2(prevPoint - segment.p0());
        double distanceToP3 = length2(prevPoint - segment.p3());

        if (distanceToP3 < distanceToP0)
        {
            BezierSegment dstSegment{
              segment.p3(), segment.p2(), segment.p1(), segment.p0()
            };
            dstSpline.appendSegment(dstSegment);
            prevPoint = dstSegment.p3();
        }
        else
        {
            dstSpline.appendSegment(segment);
            prevPoint = segment.p3();
        }
    }

    return dstSpline;
}
