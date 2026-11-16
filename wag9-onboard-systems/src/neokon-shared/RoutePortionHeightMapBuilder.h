#pragma once

#include "ExtendedIntervalBound.h"
#include "MultiLineHeightMap.h"
#include "RailwayLines.h"
#include "RoutePosition.h"

#include <memory>
#include <optional>
#include <vector>

/**
 * Generates an ordered sequence of height points corresponding to the provided route.
 *
 * Routes spanning multiple railway lines are supported.
 */
class RoutePortionHeightMapBuilder
{
public:
    struct HeightPoint
    {
        /**
         * @see processNextLocation()
         */
        double relativePositionM;

        double heightAboveSeaLevelM;
    };

    /**
     * Constructs the object.
     *
     * The @p routePortionLowerBoundType and @p routePortionUpperBoundType
     * give you control over which exactly height map control points around the
     * start and the end position of the route portion will or will not be included
     * in the output. The two typical cases will be:
     * -# ExtendedIntervalBound::Exclusive for both.
     * -# ExtendedIntervalBound::ExtraInclusive for both.
     *
     * In the first case, the first and the last output points will be
     * strictly within the route portion bounds. In the second case, the
     * first and the last output points (and only them) will generally be
     * outside of the route portion bounds. That's best described visually:
     *
     * Case 1:
     * <pre>
     * I   I   I
     *   O   O
     * </pre>
     * In the picture above, I stands for an input point, and O stands for
     * the output one.
     *
     * Case 2:
     * <pre>
     *   I   I
     * O   O   O
     * </pre>
     * However, the following picture is also possible for case 2:
     * <pre>
     * I  <doesn't matter> I
     * O  <doesn't matter> O
     * </pre>
     * That is, ExtendedIntervalBound::ExtraInclusive won't add an extra control
     * point to the output, if there is a control point at exactly the boundary point.
     *
     * Also ExtendedIntervalBound::ExtraInclusive may not be able to add an extra
     * control point if it's physically not there, for example if the height map
     * doesn't cover the whole railway line. This class only outputs existing control
     * points, it doesn't synthesize new ones.
     */
    RoutePortionHeightMapBuilder(
        std::shared_ptr<RailwayLines const> railwayLines,
        std::shared_ptr<MultiLineHeightMap const> routeHeightMap,
        ExtendedIntervalBound routePortionLowerBoundType,
        ExtendedIntervalBound routePortionUpperBoundType);

    /**
     * The area to cover with a height map is given as an ordered sequence of RoutePosition
     * instances. We call them input points. Whenever you are crossing from one RailPath to
     * another, you have to provide two route positions: one at the end of the previous RailPath
     * and one at the beginning of the next one. That's because those RailPath's may belong to
     * different railway lines.
     *
     * Each RoutePosition is associated with a position on the route that is relative to
     * some unspecified reference point. Because the position is a relative one, it can be
     * negative. Relative positions are meant to be increasing as we advance along the route.
     * The reference point may be the position of a train or the beginning of a route or
     * anything else. Relative positions with respect to the same reference point are also
     * associated with output points.
     */
    void processNextLocation(RoutePosition const& routePos, double relativePositionM);

    std::vector<HeightPoint> build()&&;

private:
    struct PendingStretch
    {
        struct Bound
        {
            double relativePositionM = 0.0;
            RoutePosition routePosition;
            ExtendedIntervalBound boundType = ExtendedIntervalBound::Exclusive;
        };

        Bound start;
        std::optional<Bound> end;

        bool isCommitable() const { return end.has_value(); }

        Bound const& lastBound() const { return end ? *end : start; }
    };

    bool tryExtendPendingStretch(RoutePosition const& routePos, double relativePositionM);

    void commitPendingStretch();

    void startNewStretch(RoutePosition const& routePos,
        double relativePositionM, ExtendedIntervalBound boundType);

    void maybeOutputHeightPoint(
        double relativePositionM, double heightAboveSeaLevelM);

    std::shared_ptr<RailwayLines const> m_worldParts;

    std::shared_ptr<MultiLineHeightMap const> m_routeHeightMap;

    ExtendedIntervalBound m_routePortionLowerBoundType;
    ExtendedIntervalBound m_routePortionUpperBoundType;

    std::vector<HeightPoint> m_orderedHeightPoints;

    std::optional<PendingStretch> m_pendingStretch;
};
