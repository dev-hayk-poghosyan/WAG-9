#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <optional>
#include <utility>
#include <vector>
#include <type_traits>

/**
 * Subdivies the axis of real numbers into non-overlaping intervals and associates
 * a property object with each interval.
 */
template<typename Property>
class IntervalPropertyMap
{
public:
    /**
     * Assigns a property to an interval of real values.
     *
     * We support both the "startPos == stopPos" and the "startPos > stopPos" situations.
     * The former situation is a no-op, and the in the latter we simply swap startPos and endPos.
     *
     * Overwriting or partially overwriting existing intervals with a new one is allowed.
     * If there is a gap between the existing intervals and the new one, a gap interval
     * is created and its property is set to a default-constructed Property object.
     */
    void setPropertyOnInterval(
        Property const& property, double startPos, double stopPos);

    /**
     * Returns the property at the given position.
     *
     * If the position is not within an interval, a default-constructed
     * Property object is returned.
     *
     * If the position is on an interval boundary, @p knotPropertyResolver
     * is called to resolve the ambiguity and its return value is returned.
     *
     * The signature of @p knotPropertyResolver shall be compatible with this one:
     * @code
     * Property(std::optional<Property> const& lhs, std::optional<Property> const& rhs);
     * @endcode
     *
     * Either one or both of @p lhs and @p rhs will be set. When one of them
     * is not set, that means the @p position is the beginning of the first
     * interval or the end of the last one.
     */
    template<typename F>
    Property propertyAt(double position, F&& knotPropertyResolver) const;

    /**
     * This is a simplified version of the other overload.
     *
     * It's equivalent to calling the other overload with a knot property resolver
     * implementing the following policy:
     * @li If only @p lhs or @p rhs is set, that interval's property is used.
     * @li If both @p lhs and @p rhs are set, the @p rhs interval's property is used.
     */
    Property propertyAt(double position) const;

    /**
     * Returns a property whose value is not equal to the value of
     * the property in the position @position.
     * The property is searched in the direction specified in the @p direction parameter.
     * @return found property and distance from position to the
     * begining of the interval with the found property.
     */
    template<typename F>
    std::pair<Property, double> propertyAfter(double position,
        int direction, F&& propertToValueConverter) const;

    /**
     * Calls @p sink on each interval we hold, in the correct order.
     *
     * The signature of @p sink shall be compatible with this one:
     * @code
     * void(Property const& property, double startPos, double stopPos);
     * @endcode
     */
    template<typename F>
    void listOrderedIntervals(F&& sink) const;

    /**
     * Lists the properties and their boundaries encountered when moving from @p fromPos
     * to @p toPos.
     *
     * The intervals will be reported through @p sink.
     * The signature of @p sink shall be compatible with this one:
     * @code
     * void(Property const& property, double startPos, double stopPos);
     * @endcode
     *
     * The @p startPos and @p stopPos indicate where the property in question starts
     * and stops when moving from @p fromPos to @p toPos. So, @p startPos will be
     * greater than @p stopPos if @p fromPos is greater than @p toPos. Also, the
     * [startPos, stopPos] interval shall be clamped by the [fromPos, toPos] one.
     * When clamping [startPos, stopPos] by [fromPos, toPos] results in empty or single-point
     * intervals, they are not reported. In other worlds, @p startPos will never be equal to
     * @p stopPos.
     *
     * When we don't store a property explicitly for some parts (or the whole) of the
     * [fromPos, toPos] range, we still report those intervals, passing a default-constructed
     * Property instance.
     */
    template<typename F>
    void listPropertiesAlongTheWay(double fromPos, double toPos, F&& sink) const;

private:
    template<typename Methods, typename F>
    void listPropertiesAlongTheWayImpl(
        double fromPos, double toPos,
        double lowerBoundPos, double upperBoundPos,
        Methods m, F&& sink) const;

    /**
     * The boundaries of intervals.
     */
    std::vector<double> m_orderedKnots;

    /**
     * m_intervalProperties[i] corresponds to the interval between m_orderedKnots[i]
     * and m_orderedKnots[i + 1].
     */
    std::vector<Property> m_intervalProperties;
};


template<typename Property>
void
IntervalPropertyMap<Property>::setPropertyOnInterval(
    Property const& property, double startPos, double stopPos)
{
    assert(std::isfinite(startPos));
    assert(std::isfinite(stopPos));

    if (startPos == stopPos)
    {
        return;
    }
    else if (stopPos < startPos)
    {
        std::swap(startPos, stopPos);
    }

    std::vector<double> newOrderedKnots;
    std::vector<Property> newIntervalProperties;

    // The number of knots may grow or shrink, but it won't grow by more than 2.
    newOrderedKnots.reserve(m_orderedKnots.size() + 2);

    // The number of intervals may grow or shrink, but it won't grow by more than 2.
    // That's because the number of intervals is one less than the number of knots,
    // except the situation where we have no knots and no intervals. So, if the number
    // of knots grows by 2, the number of intervals groups by 2 as well (or by 1, if
    // there were no knots or intervals).
    newIntervalProperties.reserve(m_intervalProperties.size() + 2);

    size_t const numOldKnots = m_orderedKnots.size();
    size_t oldKnotIdx = 0;

    // Process old knots that are strictly before startPos.
    for (; oldKnotIdx < numOldKnots; ++oldKnotIdx)
    {
        double const oldKnotPos = m_orderedKnots[oldKnotIdx];
        if (!(oldKnotPos < startPos))
        {
            break;
        }

        newOrderedKnots.push_back(oldKnotPos);

        if (oldKnotIdx > 0)
        {
            newIntervalProperties.push_back(m_intervalProperties[oldKnotIdx - 1]);
        }
    }

    if (oldKnotIdx == numOldKnots)
    {
        // We ran out of existing knots, which means our new interval is to the right
        // of all the existing ones, if any.

        if (numOldKnots > 0)
        {
            // There were existing intervals, which means there is a gap between them
            // and our new interval. Therefore, we need to insert a default-constructed
            // property for that gap.
            newIntervalProperties.emplace_back();
        }
    }
    else // We do have some unprocessed existing knots.
    {
        double const nextOldKnotPos = m_orderedKnots[oldKnotIdx];
        assert(startPos <= nextOldKnotPos);

        if (startPos == nextOldKnotPos)
        {
            if (oldKnotIdx > 0)
            {
                // Bring in the property for the interval ending at startPos.
                newIntervalProperties.push_back(m_intervalProperties[oldKnotIdx - 1]);
            }

            ++oldKnotIdx; // Skip the old knot at exactly startPos.
        }
        else // The next knot is beyond startPos
        {
            assert(startPos < nextOldKnotPos);

            if (oldKnotIdx > 0)
            {
                assert(m_orderedKnots[oldKnotIdx - 1] < startPos);

                // This means startPos is strictly inside the bounds of the existing interval
                // between oldKnotIdx - 1 and oldKnotIdx. Let's call that interval "A".
                // By "strictly inside" we mean it's not on the interval's boundary.
                // So, we trim A to end at startPos. We still need to bring in its property.
                newIntervalProperties.push_back(m_intervalProperties[oldKnotIdx - 1]);
            }
        }
    }

    newOrderedKnots.push_back(startPos);

    for (; oldKnotIdx < numOldKnots && !(stopPos < m_orderedKnots[oldKnotIdx]); ++oldKnotIdx)
    {
        // Skip old knots that are before or at exatly stopPos.
    }

    newOrderedKnots.push_back(stopPos);
    newIntervalProperties.push_back(property);

    if (oldKnotIdx == 0 && oldKnotIdx < numOldKnots)
    {
        // There is a gap between stopPos and the first existing interval.
        // Therefore, we have to insert a default-constructed property for that gap.
        newIntervalProperties.emplace_back();
    }

    // Process the remaining old knots that are strictly after stopPos.
    for (; oldKnotIdx < numOldKnots; ++oldKnotIdx)
    {
        double const oldKnotPos = m_orderedKnots[oldKnotIdx];
        assert(stopPos < oldKnotPos);

        newOrderedKnots.push_back(oldKnotPos);

        if (oldKnotIdx > 0)
        {
            newIntervalProperties.push_back(m_intervalProperties[oldKnotIdx - 1]);
        }
    }

    assert(newOrderedKnots.size() == newIntervalProperties.size() + 1);

    // Replace the existing state with the new one.
    m_orderedKnots = std::move(newOrderedKnots);
    m_intervalProperties = std::move(newIntervalProperties);
}

template<typename Property>
template<typename F>
Property
IntervalPropertyMap<Property>::propertyAt(double position, F&& knotPropertyResolver) const
{
    assert(std::isfinite(position));

    // Find the first knot that's at or beyond our position.
    auto const rhsKnot = std::lower_bound(m_orderedKnots.begin(), m_orderedKnots.end(), position);
    if (rhsKnot == m_orderedKnots.end())
    {
        // This means our position is past the last interval (if any).
        return Property();
    }

    size_t const rhsKnotIdx = rhsKnot - m_orderedKnots.begin();

    if (*rhsKnot == position)
    {
        // Our position is exactly at a knot.
        return knotPropertyResolver(
            rhsKnotIdx > 0
            ? std::optional<Property>(m_intervalProperties[rhsKnotIdx - 1])
            : std::optional<Property>(),
            rhsKnotIdx < m_intervalProperties.size()
            ? std::optional<Property>(m_intervalProperties[rhsKnotIdx])
            : std::optional<Property>());
    }

    // At this point we can be certain rhsKnot points to a knot at a greater position that ours.

    if (rhsKnotIdx == 0)
    {
        // Our position precedes the very first interval.
        return Property();
    }

    return m_intervalProperties[rhsKnotIdx - 1];
}

template<typename Property>
template<typename F>
std::pair<Property, double>
IntervalPropertyMap<Property>::propertyAfter(double position, int direction, F&& sink) const
{
    assert(std::isfinite(position));

    // Find the first knot that's beyond our position.
    auto rhsKnot = std::lower_bound(m_orderedKnots.begin(), m_orderedKnots.end(), position);
    if ((direction > 0 && rhsKnot == m_orderedKnots.end())
        || (direction < 0 && rhsKnot == m_orderedKnots.begin()))
    {
        // This means our position is past the last interval (if any).
        return std::pair{ Property(), 0 };
    }
    size_t const rhsKnotIdx = rhsKnot - m_orderedKnots.begin();
    int step = (direction < 0) ? -1 : 1;
    std::invoke_result_t<decltype(sink), Property> initPropertyValue;
    std::size_t startIdx = 0;

    if (direction > 0)
    {
        initPropertyValue = (rhsKnotIdx > 0)
            ? (startIdx = rhsKnotIdx - 1, sink(m_intervalProperties[rhsKnotIdx - 1]))
            : (startIdx = 0, sink(Property()));
    }
    else
    {
        initPropertyValue = (rhsKnotIdx > m_intervalProperties.size())
            ? (startIdx = m_intervalProperties.size() - 1, sink(Property()))
            : (startIdx = rhsKnotIdx - 1, sink(m_intervalProperties[rhsKnotIdx - 1]));
    }

    for (std::size_t i = startIdx; i < m_intervalProperties.size(); i += step)
    {

        auto const& property = m_intervalProperties[i];
        auto propertyValue = sink(property);
        if (propertyValue != initPropertyValue)
        {
            int knotIdx = (direction < 0) ? i + 1 : i;
            return std::pair{ property, (m_orderedKnots[knotIdx] - position) * step };
        }

    }
    return std::pair{ Property(), 0 };
}

template<typename Property>
Property
IntervalPropertyMap<Property>::propertyAt(double position) const
{
    class DefaultKnotPropertyResolver
    {
    public:
        Property operator()(
            std::optional<Property> const& lhs, std::optional<Property> const& rhs) const
        {
            assert(lhs || rhs); // Both can also be set.

            if (rhs)
            {
                // We take the value from the right hand side interval, even when we have both
                // intervals. That is, we assume the beginning of the interval is inclusive.
                return *rhs;
            }
            
            if (lhs)
            {
                assert(lhs);
                return *lhs;
            }

            return Property{};
        }
    };

    return propertyAt(position, DefaultKnotPropertyResolver());
}

template<typename Property>
template<typename F>
void
IntervalPropertyMap<Property>::listOrderedIntervals(F&& sink) const
{
    size_t const numKnots = m_orderedKnots.size();
    for (size_t i = 1; i < numKnots; ++i)
    {
        sink(m_intervalProperties[i - 1], m_orderedKnots[i - 1], m_orderedKnots[i]);
    }
}

namespace list_properties_along_the_way_impl
{

struct IncreasingPositionsMethods
{
    template<typename T>
    bool precedesFollows(T preceding, T following) const
    {
        return preceding < following;
    }

    template<typename T>
    T encounteredFirst(T pos1, T pos2) const
    {
        return std::min(pos1, pos2);
    }

    template<typename T>
    T encounteredLast(T pos1, T pos2) const
    {
        return std::max(pos1, pos2);
    }

    template<typename T>
    T encounteredFirstLoHi(T lowPos, T highPos) const
    {
        return lowPos;
    }

    template<typename T>
    T encounteredLastLoHi(T lowPos, T highPos) const
    {
        return highPos;
    }
};

struct DecreasingPositionsMethods
{
    template<typename T>
    bool precedesFollows(T preceding, T following) const
    {
        return preceding > following;
    }

    template<typename T>
    T encounteredFirst(T pos1, T pos2) const
    {
        return std::max(pos1, pos2);
    }

    template<typename T>
    T encounteredLast(T pos1, T pos2) const
    {
        return std::min(pos1, pos2);
    }

    template<typename T>
    T encounteredFirstLoHi(T lowPos, T highPos) const
    {
        return highPos;
    }

    template<typename T>
    T encounteredLastLoHi(T lowPos, T highPos) const
    {
        return lowPos;
    }
};

} // namespace list_properties_along_the_way_impl

template<typename Property>
template<typename F>
void
IntervalPropertyMap<Property>::listPropertiesAlongTheWay(
    double fromPos, double toPos, F&& sink) const
{
    // We can't make the classes from this namespace function-local,
    // as template methods are not allowed in local classes.
    // We could make them inner classes, but that's not ideal either,
    // as they don't depend on the outer class.
    using namespace list_properties_along_the_way_impl;

    assert(std::isfinite(fromPos));
    assert(std::isfinite(toPos));

    if (fromPos < toPos)
    {
        double const lowerBoundPos = fromPos;
        double const upperBoundPos = toPos;

        listPropertiesAlongTheWayImpl(
            fromPos, toPos, lowerBoundPos, upperBoundPos,
            IncreasingPositionsMethods(),
            std::forward<F>(sink));
    }
    else if (fromPos > toPos)
    {
        double const lowerBoundPos = toPos;
        double const upperBoundPos = fromPos;

        listPropertiesAlongTheWayImpl(
            fromPos, toPos, lowerBoundPos, upperBoundPos,
            DecreasingPositionsMethods(),
            std::forward<F>(sink));
    }
}

template<typename Property>
template<typename M, typename F>
void
IntervalPropertyMap<Property>::listPropertiesAlongTheWayImpl(
    double const fromPos, double const toPos,
    double const lowerBoundPos, double const upperBoundPos,
    M m, F&& sink) const
{
    if (m_intervalProperties.empty())
    {
        sink(Property(), fromPos, toPos);
        return;
    }

    size_t const firstKnotIdx = m.encounteredFirstLoHi<size_t>(0, m_orderedKnots.size() - 1);
    double const firstKnotValue = m_orderedKnots[firstKnotIdx];

    if (m.precedesFollows(fromPos, firstKnotValue))
    {
        // We need a fake interval before the real ones.
        sink(Property(), fromPos, m.encounteredFirst(toPos, firstKnotValue));
    }

    ptrdiff_t const firstIntervalIdx =
        m.encounteredFirstLoHi<ptrdiff_t>(0, m_intervalProperties.size() - 1);

    ptrdiff_t const lastIntervalIdx =
        m.encounteredLastLoHi<ptrdiff_t>(0, m_intervalProperties.size() - 1);

    // This one is a bit of a hack: it will pick +1 if distances are increasing and -1 otherwise.
    ptrdiff_t const intervalIdxDelta = m.encounteredLastLoHi<ptrdiff_t>(-1, +1);

    for (ptrdiff_t intervalIdx = firstIntervalIdx;; intervalIdx += intervalIdxDelta)
    {
        size_t const lowerBoundKnotIdx = intervalIdx;
        size_t const upperBoundKnotIdx = intervalIdx + 1;

        double const intersectionLowerBound =
            std::max(lowerBoundPos, m_orderedKnots[lowerBoundKnotIdx]);

        double const intersectionUpperBound =
            std::min(upperBoundPos, m_orderedKnots[upperBoundKnotIdx]);

        if (intersectionUpperBound > intersectionLowerBound)
        {
            sink(m_intervalProperties[intervalIdx],
                m.encounteredFirstLoHi(intersectionLowerBound, intersectionUpperBound),
                m.encounteredLastLoHi(intersectionLowerBound, intersectionUpperBound));
        }

        if (intervalIdx == lastIntervalIdx)
        {
            break;
        }
    }

    size_t const lastKnotIdx = m.encounteredLastLoHi<size_t>(0, m_orderedKnots.size() - 1);
    double const lastKnotValue = m_orderedKnots[lastKnotIdx];

    if (m.precedesFollows(lastKnotValue, toPos))
    {
        // We need a fake interval after the real ones.
        sink(Property(), m.encounteredLast(fromPos, lastKnotValue), toPos);
    }
}
