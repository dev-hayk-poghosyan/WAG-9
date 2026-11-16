#include <UnigineMathLib.h>

/**
 * Return radians angle difference between 'angleFromRad' and 'angleToRad' normalised to 0 .. M_PI
 */
double normalizedAngleFromToRad(double angleFromRad, double angleToRad);

/**
 * Return degrees angle difference bwtween 'angleFromDeg' and 'angleToDeg' normalised to 0 .. 180
 */
double normalizedAngleFromToDeg(double angleFromDeg, double angleToDeg);

/**
 * Return radians angle difference bwtween 'fromDir' and 'toDir' directions normalised to 0 .. M_PI
 */
double normalizedAngleFromToRad(const Unigine::Math::Vec3& fromDir,
    const Unigine::Math::Vec3& toDir);

/**
 * Return degrees angle difference bwtween 'fromDir' and 'toDir' directions normalised to 0 .. 180
 */
double normalizedAngleFromToDeg(const Unigine::Math::Vec3& fromDir,
    const Unigine::Math::Vec3& toDir);
