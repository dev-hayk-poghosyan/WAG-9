#include "NormalizedAngleFromTo.h"
#include <cmath>

using namespace Unigine;

namespace
{

constexpr double mathPi = 3.14159265358979323846; /* pi */
constexpr double math2Pi = mathPi * 2.;

} //Anonymous namespace

double
normalizedAngleFromToRad(double angleFromRad, double angleToRad)
{
    double res = std::fmod(angleFromRad - angleToRad, math2Pi);
    if (res < 0)
        res = res + math2Pi;
    if (res > mathPi)
        res = res - math2Pi;
    return (-res);
}

double
normalizedAngleFromToDeg(double angleFromDeg, double angleToDeg)
{
    double res = std::fmod(angleFromDeg - angleToDeg, 360.);
    if (res < 0)
        res = res + 360.;
    if (res > 180)
        res = res - 360.;
    return (-res);
}

double
normalizedAngleFromToRad(const Math::Vec3& fromDir, const Math::Vec3& toDir)
{
    double dot = Math::dot(fromDir, toDir);
    return std::acos(dot);
}

double
normalizedAngleFromToDeg(const Math::Vec3& fromDir, const Math::Vec3& toDir)
{
    return normalizedAngleFromToRad(fromDir, toDir) * Math::Consts::RAD2DEG_D;
}
