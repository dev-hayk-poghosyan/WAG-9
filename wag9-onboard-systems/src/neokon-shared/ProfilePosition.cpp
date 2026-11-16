#include "ProfilePosition.h"

#include <glm/glm.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>

ProfilePosition::ProfilePosition(HeightMapControlPoint const& singleCP)
    : m_cpBehind(singleCP)
    , m_cpAhead(singleCP)
    , m_alpha(0.0)
{
    // Only with alpha == 0 we'll get the expected results from calcHeight() and calcSlopePerMille().
}

ProfilePosition::ProfilePosition(
    HeightMapControlPoint const& cpBehind,
    HeightMapControlPoint const& cpAhead, double alpha)
    : m_cpBehind(cpBehind)
    , m_cpAhead(cpAhead)
    , m_alpha(alpha)
{
    assert(m_cpBehind.positionM != m_cpAhead.positionM);
}

double
ProfilePosition::calcHeightM() const
{
    // We use cubic interpolation on the interval [0, 1].
    // See https://www.paulinternet.nl/?page=bicubic for more details.

    double const signedSpanLengthM = m_cpAhead.positionM - m_cpBehind.positionM;

    // We evaluate the "height from alpha" function at m_alpha. Because of that,
    // our derivatives are also with respect to alpha, rather than to meters.
    double const derivBehind = m_cpBehind.slope * signedSpanLengthM;
    double const derivAhead = m_cpAhead.slope * signedSpanLengthM;

    glm::dvec4 const coeffs(
        2.0 * (m_cpBehind.heightM - m_cpAhead.heightM) + derivBehind + derivAhead,
        3.0 * (m_cpAhead.heightM - m_cpBehind.heightM) - 2.0 * derivBehind - derivAhead,
        derivBehind,
        m_cpBehind.heightM);

    glm::dvec4 const powers(m_alpha * m_alpha * m_alpha, m_alpha * m_alpha, m_alpha, 1.0);

    return glm::dot(coeffs, powers);
}

double
ProfilePosition::calcSlopePerMille() const
{
    double const signedSpanLengthM = m_cpAhead.positionM - m_cpBehind.positionM;
    if (signedSpanLengthM == 0.0)
    {
        // To avoid a division by zero later.
        return 0.0;
    }

    // We differentiate the height function with respect to alpha and evaluate it at m_alpha.
    // Because of that, our derivatives are also with respect to alpha, rather than to meters.
    double const derivBehind = m_cpBehind.slope * signedSpanLengthM;
    double const derivAhead = m_cpAhead.slope * signedSpanLengthM;

    glm::dvec3 const coeffs(
        6.0 * (m_cpBehind.heightM - m_cpAhead.heightM) + 3.0 * (derivBehind + derivAhead),
        6.0 * (m_cpAhead.heightM - m_cpBehind.heightM) - 4.0 * derivBehind - 2.0 * derivAhead,
        derivBehind);

    glm::dvec3 const powers(m_alpha * m_alpha, m_alpha, 1.0);

    // The derivative of the height function with respect to alpha.
    double const deriv = glm::dot(coeffs, powers);

    // Convert the derivative with respect to alpha units to the derivative
    // with respect to meters.
    double const tangent = deriv / std::abs(signedSpanLengthM);

    // We return the slope in per mille units.
    return tangent * 1000.0;
}
