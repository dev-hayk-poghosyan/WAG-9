#include "BezierSpline.h"
#include "CalcOffsetedBezierSegment.h"
#include "GlmUnigineInterop.h"
#include "FloatingPointCompare.h"
#include "TransformFromPositionAndAxis.h"

namespace
{
const AlignedBox<glm::dvec3> emptyAabb;
}

bool operator==(BezierSpline const& left, BezierSpline const& right)
{
    auto leftSegments = left.buildSegmentsVector();
    auto rightSegments = right.buildSegmentsVector();

    if (leftSegments.size() != rightSegments.size())
    {
        return false;
    }

    int numSegments = leftSegments.size();
    for (int i = 0; i < numSegments; i++)
    {
        if (leftSegments[i].isDifferentWith(rightSegments[i]))
        {
            return false;
        }
    }

    return true;
}

bool operator!=(BezierSpline const& left, BezierSpline const& right)
{
    return !(left == right);
}

BezierSpline::BezierSpline(BezierSpline&& other)
    : m_length(other.m_length)
    , m_segments(std::move(other.m_segments))
    , m_aabb(std::move(other.m_aabb))
{
}

BezierSpline::BezierSpline(const BezierSpline& other)
    : m_length(other.m_length)
    , m_segments(other.m_segments)
    , m_aabb(other.m_aabb)
{
}

BezierSpline::BezierSpline(const BezierSegment& segment)
{
    appendSegment(segment);
}

BezierSpline::BezierSpline(const std::vector<glm::dvec3>& points)
{
    for (int pointIdx = 1; pointIdx < points.size(); ++pointIdx)
    {
        const auto& prevPoint = points[pointIdx - 1];
        const auto& point = points[pointIdx];
        appendSegment(BezierSegment(prevPoint, (point - prevPoint) * 0.333 + prevPoint,
            (prevPoint - point) * 0.333 + point, point));
    }
}

BezierSpline& BezierSpline::operator=(BezierSpline&& other)
{
    m_length = other.m_length;
    m_segments = std::move(other.m_segments);
    m_aabb = std::move(other.m_aabb);
    return *this;
}

bool
BezierSpline::isDifferentWith(const BezierSpline& other, double epsilonSquared) const
{
    if (&other == this)
    {
        return false;
    }
    if (m_segments.size() != other.m_segments.size())
    {
        return true;
    }
    auto segments = buildSegmentsVector();
    auto otherSegments = other.buildSegmentsVector();
    for (size_t s = 0; s < segments.size(); ++s)
    {
        if (segments[s].isDifferentWith(otherSegments[s], epsilonSquared))
        {
            return true;
        }
    }
    return false;
}

std::vector<BezierSegment> BezierSpline::buildSegmentsVector() const
{
    std::vector<BezierSegment> result;
    for (auto const& [interval, segment] : m_segments)
    {
        result.emplace_back(*segment);
    }
    return result;
}

int
BezierSpline::insertPoints(const std::vector<BezierSpline::PointToInsert>& points)
{
    int result = 0;
    for (const auto& [distance, position] : points)
    {
        auto segmentAtDistanceIt = m_segments.find(distance);
        if (segmentAtDistanceIt == m_segments.end())
        {
            return false;
        }
        auto interval = segmentAtDistanceIt->first;
        auto segmentToSplit = segmentAtDistanceIt->second;
        double t =
            segmentAtDistanceIt->second->tAtDistance(distance - segmentAtDistanceIt->first.lower());
        auto newSegments = segmentToSplit->subdivide(t);
        // After changing segments, intervals become inaccurate - rebuild will be required
        newSegments[0] =
            BezierSegment(newSegments[0].p0(), newSegments[0].p1(), newSegments[0].p2(), position);
        newSegments[1] =
            BezierSegment(position, newSegments[1].p1(), newSegments[1].p2(), newSegments[1].p3());

        m_segments.erase(*segmentAtDistanceIt);

        SegmentsIntervalMap::interval_type first(interval.lower(), distance,
            interval.bounds().left());
        m_segments.insert({first, std::make_shared<BezierSegment>(newSegments[0])});

        SegmentsIntervalMap::interval_type second(distance, interval.upper(),
            boost::icl::interval_bounds(
                boost::icl::interval_bounds::_left & interval.bounds().right().bits()));
        m_segments.insert({second, std::make_shared<BezierSegment>(newSegments[1])});

        ++result;
    }

    // Rebuild intervals

    m_length = 0.;
    m_aabb = std::nullopt;
    auto updatedSegments = std::move(m_segments);
    for (const auto& [_, segment] : updatedSegments)
    {
        appendSegment(*segment);
    }

    return result;
}

int
BezierSpline::insertPoints(const std::vector<double>& distances, const ModifyPointFunc& modifyFunc)
{
    int result = 0;
    for (const auto& distance : distances)
    {
        auto segmentAtDistanceIt = m_segments.find(distance);
        if (segmentAtDistanceIt == m_segments.end())
        {
            continue;
        }
        auto interval = segmentAtDistanceIt->first;
        auto segmentToSplit = segmentAtDistanceIt->second;
        double t =
            segmentAtDistanceIt->second->tAtDistance(distance - segmentAtDistanceIt->first.lower());
        auto newSegments = segmentToSplit->subdivide(t);
        // After changing segments, intervals become inaccurate - rebuild will be required

        glm::dvec3 backTangent = newSegments[0].p2();
        glm::dvec3 position = newSegments[0].p3();
        glm::dvec3 frontTangent = newSegments[1].p1();

        modifyFunc(distance, position, backTangent, frontTangent);

        newSegments[0] =
            BezierSegment(newSegments[0].p0(), newSegments[0].p1(), backTangent, position);
        newSegments[1] =
            BezierSegment(position, frontTangent, newSegments[1].p2(), newSegments[1].p3());

        m_segments.erase(*segmentAtDistanceIt);

        SegmentsIntervalMap::interval_type first(interval.lower(), distance,
            interval.bounds().left());
        m_segments.insert({first, std::make_shared<BezierSegment>(newSegments[0])});

        SegmentsIntervalMap::interval_type second(distance, interval.upper(),
            boost::icl::interval_bounds(
                boost::icl::interval_bounds::_left & interval.bounds().right().bits()));
        m_segments.insert({second, std::make_shared<BezierSegment>(newSegments[1])});

        ++result;
    }

    // Rebuild intervals

    m_length = 0.;
    m_aabb = std::nullopt;
    auto updatedSegments = std::move(m_segments);
    for (const auto& [_, segment] : updatedSegments)
    {
        appendSegment(*segment);
    }

    return result;
}

double BezierSpline::appendSegment(const BezierSegment& segment)
{
    double addLength = segment.arcLength();
    if (m_segments.empty())
    {
        m_segments.insert({ boost::icl::interval<double>::closed(m_length, m_length + addLength),
            std::make_shared<BezierSegment>(segment) });
    }
    else
    {
        m_segments.insert({ boost::icl::interval<double>::left_open(m_length, m_length + addLength),
            std::make_shared<BezierSegment>(segment) });
    }
    if (!m_aabb)
    {
        m_aabb = segment.getAABB();
    }
    else
    {
        *m_aabb |= segment.getAABB();
    }
    return m_length += addLength;
}

double
BezierSpline::appendSpline(const BezierSpline& spline)
{
    for (const auto& [interval, segment] : spline.getSegmentsMap())
    {
        appendSegment(*segment);
    }
    return m_length;
}

BezierSpline BezierSpline::createOffseted(const glm::dvec3& offset) const
{
    BezierSpline result;
    for (auto const& [interval, segment] : m_segments)
    {
        result.appendSegment(calcOffsetedBezierSegment(*segment, offset));
    }

    return result;
}

BezierSpline BezierSpline::createTrimmed(double startLength, std::optional<double> endLength) const
{
    BezierSpline result;

    for (auto const& [interval, segment] : m_segments)
    {
        if (interval.upper() <= startLength)
        {
            continue;
        }
        if (endLength && interval.lower() >= *endLength)
        {
            continue;
        }
        bool trimStart = boost::icl::contains(interval, startLength);
        bool trimEnd = endLength ? boost::icl::contains(interval, *endLength) : false;
        if (trimStart || trimEnd)
        {
            BezierSegment trimmed;

            if (trimStart)
            {
                trimmed = segment->subdivide(
                    segment->tAtDistance(startLength - interval.lower())).back();

                if (trimEnd)
                {
                    trimmed = trimmed.subdivide(
                        trimmed.tAtDistance(*endLength - startLength)).front();
                }
            }
            else
            {
                trimmed = segment->subdivide(
                    segment->tAtDistance(*endLength - interval.lower())).front();
            }
            result.appendSegment(trimmed);
        }
        else
        {
            result.appendSegment(*segment);
        }
    }

    return result;
}

BezierSpline
BezierSpline::createReversed() const
{
    BezierSpline result;

    for (auto segmentIt = m_segments.rbegin(); segmentIt != m_segments.rend(); ++segmentIt)
    {
        const auto& segment = segmentIt->second;
        result.appendSegment(
            BezierSegment(segment->p3(), segment->p2(), segment->p1(), segment->p0()));
    }

    return result;
}

BezierSpline
BezierSpline::createLocalized(std::optional<Unigine::Math::Mat4>& transform) const
{
    BezierSpline result;

    auto segments = buildSegmentsVector();
    if (!transform.has_value())
    {
        Unigine::Math::Vec3 startPoint = fromGlm(segments.front().p0());
        Unigine::Math::Vec3 endPoint = fromGlm(segments.back().p3());
        Unigine::Math::Vec3 direction = endPoint - startPoint;

        transform = transformFromTranslateAndDirectionDirYUpZ(startPoint, direction);
    }

    Unigine::Math::Mat4 invTransform = Unigine::Math::inverse(*transform);
    for (int i = 0; i < segments.size(); i++)
    {
        auto const& segment = segments[i];

        Unigine::Math::Vec3 p0 = invTransform * fromGlm(segment.p0());
        Unigine::Math::Vec3 p1 = invTransform * fromGlm(segment.p1());
        Unigine::Math::Vec3 p2 = invTransform * fromGlm(segment.p2());
        Unigine::Math::Vec3 p3 = invTransform * fromGlm(segment.p3());

        result.appendSegment(BezierSegment(toGlm(p0), toGlm(p1), toGlm(p2), toGlm(p3)));
    }

    return result;
}

const AlignedBox<glm::dvec3>& BezierSpline::getAABB() const
{
    if (m_aabb)
    {
        return *m_aabb;
    }
    return emptyAabb;
}

std::optional<glm::dvec3> BezierSpline::pointAtDistance(double distance, bool clampDistance) const
{
    double adjustedDistance = clampDistance ? clampDistanceToBounds(distance) : distance;

    auto segmentAtDistanceIt = m_segments.find(adjustedDistance);
    if (segmentAtDistanceIt == m_segments.end())
    {
        return std::nullopt;
    }
    double t = segmentAtDistanceIt->second->tAtDistance(adjustedDistance -
        segmentAtDistanceIt->first.lower());
    return segmentAtDistanceIt->second->eval(t);
}

std::optional<glm::dvec3> BezierSpline::unitTangentAtDistance(double distance, bool clampDistance) const
{
    double adjustedDistance = clampDistance ? clampDistanceToBounds(distance) : distance;

    auto segmentAtDistanceIt = m_segments.find(adjustedDistance);
    if (segmentAtDistanceIt == m_segments.end())
    {
        return std::nullopt;
    }
    double t = segmentAtDistanceIt->second->tAtDistance(adjustedDistance -
        segmentAtDistanceIt->first.lower());
    return segmentAtDistanceIt->second->unitTangentAt(t);
}

std::optional<Unigine::Math::Mat4> BezierSpline::calcSplineMatrixAtDistance(double distance,
    bool clampDistance, TransformOrthogonalization orthogonalization) const
{
    double adjustedDistance = clampDistance ? clampDistanceToBounds(distance) : distance;

    auto segmentAtDistanceIt = m_segments.find(adjustedDistance);
    if (segmentAtDistanceIt == m_segments.end())
    {
        return std::nullopt;
    }
    const BezierSegment& segment = *segmentAtDistanceIt->second;

    const double t = segment.tAtDistance(adjustedDistance - segmentAtDistanceIt->first.lower());
    return calcSplinePointMatrix(segment, t, orthogonalization);
}

std::optional<Unigine::Math::Mat4> BezierSpline::startDistanceMatrix() const
{
    if (m_segments.empty())
    {
        return std::nullopt;
    }

    auto segment =  m_segments.begin()->second;
    return calcSplinePointMatrix(*segment, 0.0);
}

std::optional<Unigine::Math::Mat4> BezierSpline::endDistanceMatrix() const
{
    if (m_segments.empty())
    {
        return std::nullopt;
    }

    auto segment = m_segments.rbegin()->second;
    return calcSplinePointMatrix(*segment, 1.0);
}

double BezierSpline::clampDistanceToBounds(double srcDistance) const
{
    return DefaultDoubleCompare::clampOpen(srcDistance, 0.0, arcLength());
}
