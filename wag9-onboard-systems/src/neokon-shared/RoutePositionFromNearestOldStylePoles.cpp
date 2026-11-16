#include "RoutePositionFromNearestOldStylePoles.h"

#include "ProtobufGlmInterop.h"

#include <glm/gtx/norm.hpp>

#include <cassert>
#include <cmath>
#include <utility>

RoutePositionFromNearestOldStylePoles::RoutePositionFromNearestOldStylePoles(
    proto::ObjectsOfInterest const& objectsOfInterest)
    : m_storage()
    , m_index(2, m_storage)
{
    for (proto::OldStylePole const& protoPole : objectsOfInterest.old_style_poles())
    {
        m_storage.poles.emplace_back(protoPole);
    }

    m_index.buildIndex();
}

std::optional<RoutePosition>
RoutePositionFromNearestOldStylePoles::resolvePosition(
    glm::dvec3 const& location, glm::dvec3* routeTangentVector) const
{
    static size_t const numResults = 2;
    size_t indices[numResults];
    double distances[numResults];

    size_t const numFound = m_index.knnSearch(
        &location[0], numResults, &indices[0], &distances[0]);

    if (numFound != numResults)
    {
        return std::nullopt;
    }

    Pole const& pole0 = m_storage.poles[indices[0]];
    Pole const& pole1 = m_storage.poles[indices[1]];

    if (discontinuityDetected(pole0, pole1))
    {
        // This means pole0 and pole1 likely belong to different lines.
        // In this case, we simply return the position of the closest of them.
        double const sqdist0 = glm::distance2(glm::dvec2(pole0.location), glm::dvec2(location));
        double const sqdist1 = glm::distance2(glm::dvec2(pole1.location), glm::dvec2(location));

        return buildRoutePosition(pole0, pole1, sqdist0 < sqdist1 ? 0.0 : 1.0, routeTangentVector);
    }

    glm::dvec3 const p0 = pole0.location;
    glm::dvec3 const p1 = pole1.location;

    // The idea is to first project "location" onto the line connecting
    // the two closest kilometer poles, and then represent the projected
    // point as a linear combination of the positions of the two poles.
    // Once we know the linear coefficients, we may use them to interpolate
    // the kilometer marks on the poles.

    // First we change the coordinate system so that p0 ends up at the origin.
    glm::dvec3 const a = location - p0;
    glm::dvec3 const b = p1 - p0;
    
    double const denom = glm::dot(b, b);
    if (denom < 1e-5)
    {
        // The two poles are too close.
        return std::nullopt;
    }

    double const projScalar = glm::dot(a, b) / denom;

    // Now the projected point is "p0 + projScalar * b" or equivalently
    // "(1 - projScalar) * p0 + projScalar * p1

    return buildRoutePosition(pole0, pole1, projScalar, routeTangentVector);
}

bool
RoutePositionFromNearestOldStylePoles::discontinuityDetected(Pole const& pole0, Pole const& pole1)
{
    double const routeDistance = glm::abs(pole0.distanceOnRoute - pole1.distanceOnRoute);
    
    // We allow 1 kilometer pole to be missing, but not two in a row.
    return routeDistance > 2.0;
}

RoutePosition
RoutePositionFromNearestOldStylePoles::buildRoutePosition(
    Pole const& pole0, Pole const& pole1, double projScalar,
    glm::dvec3* routeTangentVector) const
{
    double const distanceOnRoute = glm::mix<double>(
        pole0.distanceOnRoute, pole1.distanceOnRoute, projScalar);

    double wholeKilometers = 0.0;
    double const fractionalKilometers = std::modf(distanceOnRoute, &wholeKilometers);

    double firstDecimalPlace = 0.0;
    double const minorIntervalFraction = std::modf(fractionalKilometers * 10.0, &firstDecimalPlace);

    int const lineIdx = 0;
    int const majorIntervalIdx = (int)wholeKilometers;
    int const minorIntervalIdx = std::lround(firstDecimalPlace);
    int const numMinorIntervals = 10;

    if (routeTangentVector)
    {
        if (pole1.distanceOnRoute > pole0.distanceOnRoute)
        {
            *routeTangentVector = pole1.location - pole0.location;
        }
        else
        {
            *routeTangentVector = pole0.location - pole1.location;
        }
    }

    return RoutePosition(
        lineIdx, majorIntervalIdx, minorIntervalIdx, numMinorIntervals, minorIntervalFraction);
}


/*=========================================== Pole =============================================*/

RoutePositionFromNearestOldStylePoles::Pole::Pole(proto::OldStylePole const& pole)
    : location(protoToGlm(pole.location()))
    , distanceOnRoute(pole.position_km())
{
}
