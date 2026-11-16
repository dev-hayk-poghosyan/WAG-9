#include "BezierArcLengthCalculator.h"

#include "BezierSegment.h"

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include <algorithm>
#include <cmath>

// 9 point Gauss-Legendre coefficients, taken from:
// https://pomax.github.io/bezierinfo/legendre-gauss.html
BezierArcLengthCalculator::Sample const BezierArcLengthCalculator::m_samples[] = {
    { 0.3302393550012598, 0.0000000000000000 },
    { 0.1806481606948574, -0.8360311073266358 },
    { 0.1806481606948574, 0.8360311073266358 },
    { 0.0812743883615744, -0.9681602395076261 },
    { 0.0812743883615744, 0.9681602395076261 },
    { 0.3123470770400029, -0.3242534234038089 },
    { 0.3123470770400029, 0.3242534234038089 },
    { 0.2606106964029354, -0.6133714327005904 },
    { 0.2606106964029354, 0.6133714327005904 }
};

BezierArcLengthCalculator::BezierArcLengthCalculator(BezierSegment const& segment)
    : m_last3ControlPoints(
        segment.p1() - segment.p0(),
        segment.p2() - segment.p0(),
        segment.p3() - segment.p0())
{
}

double
BezierArcLengthCalculator::arcLengthFromTo(double const tStart, double const tEnd) const
{
    double sum = 0.0;

    for (Sample const& sample : m_samples)
    {
        double const xTStartToTEnd = glm::mix(tStart, tEnd, (sample.xMinusOneToOne + 1.0) * 0.5);
        sum += sample.weight * glm::length(splineGradientVector(xTStartToTEnd));
    }

    return 0.5 * (tEnd - tStart) * sum;
}

double
BezierArcLengthCalculator::tForDistance(double const distance) const
{
    auto const distanceErrorFunction = [this, distance](double t)
    {
        return arcLengthFromZero(t) - distance;
    };

    if (distance <= 0.0)
    {
        return 0.0;
    }

    double const totalArcLength = arcLengthFromTo(0.0, 1.0);
    if (distance >= totalArcLength)
    {
        return 1.0;
    }
    else if (totalArcLength < 1e-6)
    {
        // To prevent division by totalArcLength
        return 0.0;
    }

    double t = distance / totalArcLength;
    double tMin = 0.0;
    double tMax = 1.0;

    // Normally, the following inequalities hold:
    //
    // errorFunction(tMin).val <= 0.0
    // errorFunction(tMax).val >= 0.0
    //
    // However, due to numerical issues that may not always be true.

    for (int i = 0; i < 12; ++i)
    {
        auto const err = distanceErrorFunction(t);

        if (std::abs(err.val) < distance * 1e-9)
        {
            // Accurate enough already.
            break;
        }

        if (err.val <= 0.0)
        {
            tMin = std::max(tMin, t);
        }

        if (err.val >= 0.0)
        {
            tMax = std::min(tMax, t);
        }

        // Newton's method iteration.
        t -= err.val / err.deriv;

        if (!std::isfinite(t) || t < tMin || t > tMax)
        {
            // Fall back to bisection method.
            t = tMin + 0.5 * (tMax - tMin);
        }
    }

    return t;
}

BezierArcLengthCalculator::EvaluatedFunction
BezierArcLengthCalculator::arcLengthFromZero(double tEnd) const
{
    // The derivative being 1.0 means it's a variable not a constant!
    EvaluatedFunction const tEndVar(tEnd, 1.0);

    EvaluatedFunction sum;

    for (Sample const& sample : m_samples)
    {
        double const tScalingFactor = (sample.xMinusOneToOne + 1.0) * 0.5;
        glm::tvec3<EvaluatedFunction> const grad = splineGradientVector(tEndVar * tScalingFactor);
        sum += sample.weight * vectorLength(grad);
    }

    return sum * 0.5 * tEndVar;
}

glm::dvec3
BezierArcLengthCalculator::splineGradientVector(double t) const
{
    return m_last3ControlPoints * controlPointWeightsForSplineGradient(t);
}

glm::tvec3<BezierArcLengthCalculator::EvaluatedFunction>
BezierArcLengthCalculator::splineGradientVector(EvaluatedFunction t) const
{
    glm::tvec3<EvaluatedFunction> const weights = controlPointWeightsForSplineGradient(t);

    // Unfortunately, glm doesn't allow us to multiply matrix by a vector for non-numeric types.

    auto weightedControlPoint = [](glm::dvec3 const& cp, EvaluatedFunction w)
    {
        return glm::tvec3<EvaluatedFunction>(cp.x * w, cp.y * w, cp.z * w);
    };

    glm::tvec3<EvaluatedFunction> res = weightedControlPoint(m_last3ControlPoints[0], weights.x);
    res += weightedControlPoint(m_last3ControlPoints[1], weights.y);
    res += weightedControlPoint(m_last3ControlPoints[2], weights.z);

    return res;
}

template<typename T>
glm::tvec3<T>
BezierArcLengthCalculator::controlPointWeightsForSplineGradient(T const t) const
{
    T const t2 = t * t;

    return glm::tvec3<T>(
        9.0 * t2 - 12.0 * t + 3.0,
        -9.0 * t2 + 6.0 * t,
        3.0 * t2
    );
}

BezierArcLengthCalculator::EvaluatedFunction
BezierArcLengthCalculator::vectorLength(glm::tvec3<EvaluatedFunction> const& vec)
{
    return sqrt(vectorSquaredLength(vec));
}

BezierArcLengthCalculator::EvaluatedFunction
BezierArcLengthCalculator::vectorSquaredLength(glm::tvec3<EvaluatedFunction> const& vec)
{
    return square(vec.x) + square(vec.y) + square(vec.z);
}

BezierArcLengthCalculator::EvaluatedFunction
BezierArcLengthCalculator::square(EvaluatedFunction f)
{
    return EvaluatedFunction(f.val * f.val, 2.0 * f.val * f.deriv);
}

BezierArcLengthCalculator::EvaluatedFunction
BezierArcLengthCalculator::sqrt(EvaluatedFunction f)
{
    double const sqrtVal = std::sqrt(f.val);
    return EvaluatedFunction(sqrtVal, 0.5 * f.deriv / sqrtVal);
}

BezierArcLengthCalculator::EvaluatedFunction&
BezierArcLengthCalculator::EvaluatedFunction::operator+=(EvaluatedFunction f)
{
    val += f.val;
    deriv += f.deriv;
    return *this;
}

BezierArcLengthCalculator::EvaluatedFunction&
BezierArcLengthCalculator::EvaluatedFunction::operator-=(EvaluatedFunction f)
{
    val -= f.val;
    deriv -= f.deriv;
    return *this;
}

BezierArcLengthCalculator::EvaluatedFunction&
BezierArcLengthCalculator::EvaluatedFunction::operator*=(double scalar)
{
    val *= scalar;
    deriv *= scalar;
    return *this;
}

BezierArcLengthCalculator::EvaluatedFunction&
BezierArcLengthCalculator::EvaluatedFunction::operator*=(EvaluatedFunction f)
{
    deriv = val * f.deriv + deriv * f.val;
    val *= f.val;
    return *this;
}
