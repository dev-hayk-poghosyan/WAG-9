#include "RoutePortionHeightMapBuilder.h"

#include <cassert>
#include <cmath>
#include <stdexcept>
#include <utility>

RoutePortionHeightMapBuilder::RoutePortionHeightMapBuilder(
    std::shared_ptr<RailwayLines const> railwayLines,
    std::shared_ptr<MultiLineHeightMap const> routeHeightMap,
    ExtendedIntervalBound routePortionLowerBoundType,
    ExtendedIntervalBound routePortionUpperBoundType)
    : m_worldParts(std::move(railwayLines))
    , m_routeHeightMap(std::move(routeHeightMap))
    , m_routePortionLowerBoundType(routePortionLowerBoundType)
    , m_routePortionUpperBoundType(routePortionUpperBoundType)
{
    assert(m_worldParts);
    assert(m_routeHeightMap);
}

void
RoutePortionHeightMapBuilder::processNextLocation(
    RoutePosition const& routePos, double relativePositionM)
{
    assert(!m_pendingStretch ||
        m_pendingStretch->lastBound().relativePositionM <= relativePositionM);

    if (!m_pendingStretch)
    {
        assert(m_orderedHeightPoints.empty());
        startNewStretch(routePos, relativePositionM, m_routePortionLowerBoundType);
    }
    else if (!tryExtendPendingStretch(routePos, relativePositionM))
    {
        if (m_pendingStretch && m_pendingStretch->isCommitable())
        {
            commitPendingStretch();
        }

        startNewStretch(routePos, relativePositionM, ExtendedIntervalBound::Inclusive);
    }
}

std::vector<RoutePortionHeightMapBuilder::HeightPoint>
RoutePortionHeightMapBuilder::build()&&
{
    if (m_pendingStretch && m_pendingStretch->isCommitable())
    {
        m_pendingStretch->end->boundType = m_routePortionUpperBoundType;
        commitPendingStretch();
    }

    return std::move(m_orderedHeightPoints);
}

bool
RoutePortionHeightMapBuilder::tryExtendPendingStretch(
    RoutePosition const& routePos, double relativePositionM)
{
    assert(m_pendingStretch);

    if (routePos.lineIndex() != m_pendingStretch->start.routePosition.lineIndex())
    {
        // Railway line changed - can't extend.
        return false;
    }

    m_pendingStretch->end = PendingStretch::Bound{
        relativePositionM,
        routePos,
        ExtendedIntervalBound::Inclusive
    };

    return true;
}

void
RoutePortionHeightMapBuilder::commitPendingStretch()
{
    assert(m_pendingStretch);
    assert(m_pendingStretch->isCommitable());

    unsigned const lineIdx = m_pendingStretch->start.routePosition.lineIndex();
    if (lineIdx >= m_worldParts->lines().size())
    {
        throw std::logic_error("RoutePortionHeightMapBuilder: Invalid line index encountered");
    }

    std::shared_ptr<RailwayLine const> const& line = m_worldParts->lines()[lineIdx];

    double const stretchStartPositionM = line->distanceUnit().toMeters(
        m_pendingStretch->start.routePosition.distanceOnRoute());

    double const stretchEndPositionM = line->distanceUnit().toMeters(
        m_pendingStretch->end->routePosition.distanceOnRoute());

    bool const advancingTowardsIncreasingKilometers = stretchEndPositionM >= stretchStartPositionM;

    m_routeHeightMap->listControlPointsBetween(
        m_pendingStretch->start.routePosition,
        m_pendingStretch->end->routePosition,
        m_pendingStretch->start.boundType,
        m_pendingStretch->end->boundType,
        [this, stretchStartPositionM, advancingTowardsIncreasingKilometers](
            HeightMapControlPoint cp)
    {
        double positionDeltaM = cp.positionM - stretchStartPositionM;
        if (!advancingTowardsIncreasingKilometers)
        {
            // We can't simply take the absolute value here, as in some cases we want
            // this value to be negative, for instance the very first control point
            // in a route will generally be prior to the very first location passed
            // to processNextLocation(). In other words, it will have a negative offset
            // from the first location.
            positionDeltaM = -positionDeltaM;
        }

        double const relativeDistanceM = m_pendingStretch->start.relativePositionM + positionDeltaM;

        maybeOutputHeightPoint(relativeDistanceM, cp.heightM);
    });

    // It may seem a better idea to make the endpoint of the current stretch
    // to be the starting point of the next one. However, we only call commitPendingStretch()
    // in two cases: when tryExtendPendingStretch() failed, and in build(), where
    // we know there won't be any more stretches. Therefore, removing the pending stretch
    // completely is the right thing to do.
    m_pendingStretch = std::nullopt;
}

void
RoutePortionHeightMapBuilder::startNewStretch(
    RoutePosition const& routePos, double relativePositionM, ExtendedIntervalBound boundType)
{
    m_pendingStretch = PendingStretch{
        {
            relativePositionM,
            routePos,
            boundType
        }
    };
}

void
RoutePortionHeightMapBuilder::maybeOutputHeightPoint(
    double relativePositionM, double heightAboveSeaLevelM)
{
    auto tooCloseToPreviousHeightPoint = [this, relativePositionM]()
    {
        if (m_orderedHeightPoints.empty())
        {
            return false;
        }

        if (relativePositionM - m_orderedHeightPoints.back().relativePositionM < 1.0)
        {
            return true;
        }

        return false;
    };

    if (tooCloseToPreviousHeightPoint())
    {
        // A gap of zero may cause a division by zero later, and in general, very small gaps
        // are not good to have.
        return;
    }

    m_orderedHeightPoints.push_back(HeightPoint{ relativePositionM, heightAboveSeaLevelM });
}
