#include "DirectedRoutePosition.h"

#if 0
DirectedRoutePosition
DirectedRoutePosition::translatedByMeters(double amountMeters) const
{
    return DirectedRoutePosition(
        m_positionKM + amountMeters * (m_facingIncreasingKilometers ? 0.001 : -0.001),
        m_facingIncreasingKilometers);
}
#endif
#if 0
double
DirectedRoutePosition::signedDistanceInMetersTo(double targetPositionKM) const
{
    return (targetPositionKM - m_positionKM) * (m_facingIncreasingKilometers ? 1000.0 : -1000.0);
}
#endif
