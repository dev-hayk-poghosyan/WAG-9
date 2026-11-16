#include "SingleLineRoutePositionResolver.h"

#include "ProtobufGlmInterop.h"

#include <boost/format.hpp>

#include <cassert>
#include <sstream>
#include <utility>

SingleLineRoutePositionResolver::SingleLineRoutePositionResolver(
    unsigned lineIndex, std::vector<proto::Pole> const& poles)
    : m_lineIndex(lineIndex)
    , m_storage()
    , m_index(2, m_storage)
{
    for (auto const& protoPole : poles)
    {
        if (protoPole.type_case() != proto::Pole::kMajor &&
            protoPole.type_case() != proto::Pole::kMinor)
        {
            throw std::logic_error(str(boost::format(
                "Pole \"%1%\" has unknown or unspecified type.")
                % protoPole.node_name()));
        }

        if (protoPole.line_idx() != m_lineIndex)
        {
            throw std::invalid_argument(str(boost::format(
                "Pole \"%1%\" has invalid line index (expected %1%, got %2%)")
                 % protoPole.line_idx() % m_lineIndex));
        }

        m_storage.poles.emplace_back(protoPole);
    }

    // We need poles to be without gaps. Have to sort them to check it.
    std::sort(m_storage.poles.begin(), m_storage.poles.end(), [](Pole const& lhs, Pole const& rhs)
    {
        return lhs.orderToken() < rhs.orderToken();
    });

    for (std::size_t i = 0; i + 1 < m_storage.poles.size(); ++i)
    {
        proto::Pole const& prevPole = m_storage.poles[i].protoPole;
        proto::Pole const& nextPole = m_storage.poles[i + 1].protoPole;

        auto const validationFail = [&prevPole, &nextPole](char const* msg)
        {
            throw std::logic_error(
                str(boost::format("%1% (\"%2%\" -> \"%3%\").")
                % msg % prevPole.node_name() % nextPole.node_name()));
        };

        if (prevPole.type_case() == proto::Pole::kMinor)
        {
            if (nextPole.type_case() == proto::Pole::kMajor)
            {
                if (prevPole.minor().minor_pole_number() + 1 !=
                    prevPole.minor().num_minor_intervals())
                {
                    validationFail(
                        "Invalid minor pole index at minor pole -> major pole transition");
                }

                // Last minor pole increases major route position.
                if (prevPole.minor().major_interval_idx() != nextPole.major().route_position() - 1)
                {
                    validationFail("A missing or duplicated major pole detected "
                        "at minor pole -> major pole transition");
                }
            }
            // Two adjacent minor poles.
            else
            {
                if (prevPole.minor().minor_pole_number() != nextPole.minor().minor_pole_number() - 1)
                {
                    validationFail("A missing or duplicated minor pole encountered");
                }

                if (prevPole.minor().num_minor_intervals() != nextPole.minor().num_minor_intervals())
                {
                    validationFail(
                        "Disagreement on the number of minor intervals in a major interval");
                }
            }
        }
        else if (prevPole.type_case() == proto::Pole::kMajor)
        {
            if (nextPole.type_case() == proto::Pole::kMinor)
            {
                if (prevPole.major().route_position() != nextPole.minor().major_interval_idx())
                {
                    validationFail("Missing major pole detected at "
                        "major pole -> minor pole transition");
                }

                if (nextPole.minor().minor_pole_number() != 1)
                {
                    validationFail(
                        "The first minor pole following a major pole must have the number of 1");
                }
            }
            else
            {
                if (prevPole.major().route_position() != nextPole.major().route_position() - 1)
                {
                    validationFail("Missing major pole detected at "
                        "major pole -> major pole transition");
                }
            }
        }
    }

    m_index.buildIndex();
}

bool
SingleLineRoutePositionResolver::likelyBelongsToThisLine(glm::dvec2 const& point) const
{
    /**
     * To check whether a point belongs to a our line,
     * we take the two nearest poles and check whether they form an obtuse angle with the point.
     *
     * However, if the distance between the point from one of the poles is too small it may happen
     * that evaluated angle is not obtuse, although the point logically can be related to our line.
     * We increase the chances of success be checking three closest poles in pairs.
     */
    static size_t const numResults = 3;
    static size_t const minNumResults = 2;

    size_t indices[numResults];
    double distances[numResults];

    size_t const numFound = m_index
        .knnSearch(&point[0], numResults, &indices[0], &distances[0]);

    if (numFound < minNumResults)
        return false;

    for (std::size_t i = 0; i < numFound - 1; ++i)
    {
        for (std::size_t j = i + 1; j < numFound; ++j)
        {
            auto const& p0 = m_storage.poles[indices[i]].location;
            auto const& p1 = m_storage.poles[indices[j]].location;

            bool const obtuseAngle = glm::dot(p0 - point, p1 - point) < 0.0;

            if (obtuseAngle)
                return true;
        }
    }

    return false;
}

std::optional<RoutePosition>
SingleLineRoutePositionResolver::resolve(
    glm::dvec2 const& location, glm::dvec2* routeTangentVector) const
{
    static size_t const numResults = 1;
    static size_t const minNumResults = 1;

    // Projects point l onto the line (p0, p1).
    // Returns the interpolation factor between p0 and p1.
    auto const project = [&location](glm::dvec2 const& p0, glm::dvec2 const& p1)
    {
        auto const a = location - p0;
        auto const b = p1 - p0;
        auto const d = glm::dot(b, b);

        return d < 1e-5 ? 0.0 : glm::dot(a, b) / d;
    };

    size_t closestIdx = 0;
    double closestDst = 0;

    if (m_index.knnSearch(&location[0], numResults, &closestIdx, &closestDst) < minNumResults)
        return std::nullopt;

    Pole const* closest = &m_storage.poles[closestIdx];

    Pole const* prev = closestIdx > 0
        ? &m_storage.poles[closestIdx - 1]
        : nullptr;

    Pole const* next = closestIdx + 1 < m_storage.poles.size()
        ? &m_storage.poles[closestIdx + 1]
        : nullptr;

    if (prev)
    {
        double const interpolationFactor = project(prev->location, closest->location);
        if (interpolationFactor >= 0 && interpolationFactor <= 1.0)
        {
            return buildRoutePosition(*prev, *closest, interpolationFactor, routeTangentVector);
        }
    }

    if (next)
    {
        double const interpolationFactor = project(closest->location, next->location);
        if (interpolationFactor >= 0 && interpolationFactor <= 1.0)
        {
            return buildRoutePosition(*closest, *next, interpolationFactor, routeTangentVector);
        }
    }

    // There are a couple of ways we can get here. Maybe we are past the end of the line,
    // or maybe we have the following situation:
    // ------ *
    //        \
    //         \
    // In both cases, the best we can do is to assume we are exactly at the "closest" pole.
    if (next)
    {
        return buildRoutePosition(*closest, *next, 0.0, routeTangentVector);
    }
    else if (prev)
    {
        return buildRoutePosition(*prev, *closest, 1.0, routeTangentVector);
    }
    else
    {
        // We can't determine a position based on a single pole,
        // as in case it's a major pole, we won't be able to fill
        // the numMinorIntervals field of RoutePosition.
        return std::nullopt;
    }
}

RoutePosition
SingleLineRoutePositionResolver::buildRoutePosition(
    Pole const& prevPole, Pole const& nextPole,
    double interpolationFactor, glm::dvec2* routeTangentVector) const
{
    proto::Pole const& protoPrev = prevPole.protoPole;
    proto::Pole const& protoNext = nextPole.protoPole;

    assert(protoPrev.type_case() == proto::Pole::TypeCase::kMinor ||
        protoPrev.type_case() == proto::Pole::TypeCase::kMajor);

    assert(protoNext.type_case() == proto::Pole::TypeCase::kMinor ||
        protoNext.type_case() == proto::Pole::TypeCase::kMajor);

    if (routeTangentVector)
    {
        *routeTangentVector = nextPole.location - prevPole.location;
    }

    if (protoPrev.type_case() == proto::Pole::TypeCase::kMinor)
    {
        // We've got a minor -> minor or a minor -> major transition.

        return RoutePosition(
            m_lineIndex,
            protoPrev.minor().major_interval_idx(), // majorIntervalIdx
            protoPrev.minor().minor_pole_number() - 1, // minorIntervalIdx
            protoPrev.minor().num_minor_intervals(),
            interpolationFactor
        );
    }
    else if (protoNext.type_case() == proto::Pole::TypeCase::kMinor)
    {
        // We've got a major -> minor transition.

        assert(protoNext.minor().minor_pole_number() == 1);

        return RoutePosition(
            m_lineIndex,
            protoPrev.major().route_position(), // majorIntervalIdx
            protoNext.minor().minor_pole_number() - 1, // minorIntervalIdx
            protoNext.minor().num_minor_intervals(),
            interpolationFactor
        );
    }
    else
    {
        // We've got a major -> major transition.

        return RoutePosition(
            m_lineIndex,
            protoPrev.major().route_position(), // majorIntervalIdx
            0, // minorIntervalIdx
            1, // numMinorIntervals
            interpolationFactor
        );
    }
};


/*====================================== Pole ===================================*/

SingleLineRoutePositionResolver::Pole::Pole(proto::Pole const& pole)
    : location(protoToGlm(pole.location()))
    , protoPole(pole)
{
}

std::tuple<int, int>
SingleLineRoutePositionResolver::Pole::orderToken() const
{
    if (protoPole.type_case() == proto::Pole::TypeCase::kMajor)
        return std::make_tuple(protoPole.major().route_position(), 0);

    if (protoPole.type_case() == proto::Pole::TypeCase::kMinor)
        return std::make_tuple(protoPole.minor().major_interval_idx(),
            protoPole.minor().minor_pole_number());

    return std::make_tuple(-1, -1);
}
