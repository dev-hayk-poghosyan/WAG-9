#pragma once

struct HeightMapControlPoint
{
    /**
     * Corresponds to RoutePosition::distanceOnRoute(), and except it's always in meters.
     */
    double positionM;

    /**
     * The height above the sea level, in meters.
     */
    double heightM;

    /**
     * The change in position over the change in distance, at the given position, after unit
     * conversions. We don't really know this quantity at control points, but we estimate it
     * from the heights at neighboring control points. This quantity is then used to apply
     * cubic interpolation between a pair of adjacent control points.
     */
    double slope;
};
