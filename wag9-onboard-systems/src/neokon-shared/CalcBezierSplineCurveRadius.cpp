#include "CalcBezierSplineCurveRadius.h"

std::optional<double>
calcBezierSplineCurveRadius(BezierSpline const& spline, double distanceFrom, double distanceTo)
{
    double distanceCenter = (distanceFrom + distanceTo) * 0.5;
    std::optional<glm::dvec3> positionFrom = spline.pointAtDistance(distanceFrom, true);
    std::optional<glm::dvec3> positionTo = spline.pointAtDistance(distanceTo, true);
    std::optional<glm::dvec3> positionCenter = spline.pointAtDistance(distanceCenter);

    if (!positionFrom || !positionTo || !positionCenter)
    {
        return std::nullopt;
    }

    glm::dvec3 chordCenter = (*positionFrom) + (*positionTo - *positionFrom) * 0.5;
    const double chordHeight = glm::length(*positionCenter - chordCenter);
    const double chordLength = glm::length(*positionTo - *positionFrom);
    const double halfOfChordLength = chordLength / 2.0;

    if ((chordHeight * chordHeight) < std::numeric_limits<double>::epsilon())
    {
        return std::numeric_limits<double>::max();
    }

    const double nominator = (halfOfChordLength * halfOfChordLength) + (chordHeight * chordHeight);
    const double denominator = chordHeight * 2.0;
    double radius = nominator / denominator;

    glm::vec3 tangentVec = glm::normalize((*positionTo) - (*positionFrom));
    glm::vec3 normalVec = glm::normalize(chordCenter - (*positionCenter));
    tangentVec.z = normalVec.z = 0.0;
    glm::vec3 upVec = glm::cross(tangentVec, normalVec);

    radius *= glm::sign(upVec.z);

    return radius;
}
