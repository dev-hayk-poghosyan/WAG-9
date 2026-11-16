#pragma once

#include "HeightMapControlPoint.h"

class ProfilePosition
{
public:
    ProfilePosition(HeightMapControlPoint const& singleCP);

    ProfilePosition(
        HeightMapControlPoint const& cpBehind,
        HeightMapControlPoint const& cpAhead, double alpha);

    /**
     * Returns the height in either meters or feet above the sea level.
     */
    double calcHeightM() const;

    double calcSlopePerMille() const;

private:
    /**
     * @note m_cpBehind and m_cpAhead are allowed to be the same control point,
     *       but only if m_alpha is zero.
     */
    HeightMapControlPoint m_cpBehind;
    HeightMapControlPoint m_cpAhead;

    /**
     * The interpolation factor between 0 and 1 that interpolates from
     * m_cpBehind (0) to m_cpAhead (1).
     */
    double m_alpha = 0.0;
};
