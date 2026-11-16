#pragma once

#include "RoutePosition.h"

/**
 * Represents a point on a rail route plus its facing direction.
 */
class DirectedRoutePosition
{
public:
    DirectedRoutePosition(RoutePosition const& routePosition, bool facingIncreasingKilometers)
        : m_routePosition(routePosition)
        , m_facingIncreasingKilometers(facingIncreasingKilometers)
    {
    }

    RoutePosition const& routePosition() const { return m_routePosition; }

    bool facingIncreasingKilometers() const { return m_facingIncreasingKilometers; }

    /**
     * Returns the position resulting from moving from the current position either
     * forwards (positive amountMeters) or backwards (negative amountMeters).
     */
    DirectedRoutePosition translatedByMeters(double amountMeters) const;

    /**
     * Returns the translation amount to translate the current position to the desired one.
     *
     * The translation amount may be positive or negative.
     */
    double signedDistanceInMetersTo(RoutePosition const& routePosition) const;

private:
    RoutePosition m_routePosition;
    bool m_facingIncreasingKilometers;
};
