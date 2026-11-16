#include "TransformSpline.h"
#include "GlmUnigineInterop.h"
#include <glm/gtx/norm.hpp>

#include "TransformFromPositionAndAxis.h"

namespace
{

// Taken from here: https://stackoverflow.com/a/31858634
double nearestPointOffsetOnSegment(const glm::dvec3& segmentFrom, const glm::dvec3& segmentTo,
    const glm::dvec3& point)
{
    const glm::dvec3 segment = segmentTo - segmentFrom;
    const glm::dvec3 toPoint = point - segmentFrom;
    return std::clamp(glm::dot(toPoint, segment) / glm::length2(segment), 0., 1.);
}

// Taken from here: https://math.stackexchange.com/a/4289668
struct NearestPointsOffsets
{
    double segmentAOffset;
    double segmentBOffset;
};

NearestPointsOffsets nearestPointsOffsetsBetweenTwoSegments(const glm::dvec3& segmentAFrom,
    const glm::dvec3& segmentATo, const glm::dvec3& segmentBFrom, const glm::dvec3& segmentBTo)
{
    const glm::dvec3 P1 = segmentAFrom;
    const glm::dvec3 P2 = segmentBFrom;
    const glm::dvec3 V1 = segmentATo - segmentAFrom;
    const glm::dvec3 V2 = segmentBTo - segmentBFrom;

    const glm::dvec3 V21 = P2 - P1;

    const double v22 = glm::dot(V2, V2);
    const double v11 = glm::dot(V1, V1);
    const double v21 = glm::dot(V2, V1);
    const double v21_1 = glm::dot(V21, V1);
    const double v21_2 = glm::dot(V21, V2);
    const double denom = v21 * v21 - v22 * v11;

    if (std::abs(denom) < std::numeric_limits<double>::min())
    {
        return NearestPointsOffsets{ 0., -v21_1 / v21 };
    }
    return NearestPointsOffsets{
        std::clamp((v21_2 * v21 - v22 * v21_1) / denom, 0., 1.),
        std::clamp((-v21_1 * v21 + v11 * v21_2) / denom, 0., 1.)
    };
}

}//Anonymous namespace

using namespace Unigine;

TransformSpline::TransformSpline()
    : m_pointsAdaptor(m_points)
    , m_pointsTree(3 /*dim*/, m_pointsAdaptor, nanoflann::KDTreeSingleIndexAdaptorParams(10 /* max leaf */))
{
}

TransformSpline::TransformSpline(const TransformSpline& other)
    : m_pointsAdaptor(m_points)
    , m_pointsTree(3 /*dim*/, m_pointsAdaptor,
          nanoflann::KDTreeSingleIndexAdaptorParams(10 /* max leaf */))
{
    for (size_t pointIdx = 0; pointIdx < other.m_points.size(); ++pointIdx)
    {
        const auto& point = other.m_points[pointIdx];
        appendSegment(point.transform, point.distanceOffset - m_length);
    }
    m_pointsTree.buildIndex();
}

double
TransformSpline::build(const BezierSpline& bezierSpline, double stepM, double startM,
    std::optional<double> endM)
{
    double len = bezierSpline.arcLength();
    if (!endM || (*endM > len))
    {
        endM = len;
    }

    if (startM > endM)
    {
        double bak = startM;
        startM = *endM;
        endM = bak;
    }

    auto calcFunc = [&bezierSpline](double distance) -> std::optional<Unigine::Math::Mat4>
    {
        return bezierSpline.calcSplineMatrixAtDistance(distance, /*clampDistance=*/true);
    };

    return build(calcFunc, stepM, startM, *endM);
}

double
TransformSpline::build(const CalcSplineMatrixAtDistanceFunc& calcFunc, double stepM, double startM,
    double endM)
{
    if (startM > endM)
    {
        double bak = startM;
        startM = endM;
        endM = bak;
    }

    const double srcLength = endM - startM;
    const long numSteps = std::lround(srcLength / stepM);
    if (numSteps == 0)
    {
        return 0.0;
    }

    return buildNumSteps(calcFunc, numSteps, startM, endM);
}

double
TransformSpline::buildNumSteps(const BezierSpline& bezierSpline, int numSteps, double startM,
    std::optional<double> endM)
{
    double len = bezierSpline.arcLength();
    if (!endM || (*endM > len))
    {
        endM = len;
    }

    auto calcFunc = [&bezierSpline](double distance) -> std::optional<Unigine::Math::Mat4>
    {
        return bezierSpline.calcSplineMatrixAtDistance(distance, /*clampDistance=*/true);
    };

    return buildNumSteps(calcFunc, numSteps, startM, *endM);
}

double
TransformSpline::buildNumSteps(const CalcSplineMatrixAtDistanceFunc& calcFunc, int numSteps,
    double startM, double endM)
{
    m_length = 0.0;
    m_points.clear();
    m_segments.clear();

    const double srcLength = endM - startM;
    if (numSteps == 0)
    {
        return 0.0;
    }
    m_points.reserve(numSteps + 1);

    double stepM = srcLength / numSteps;
    if (endM && (endM < startM)) // For negative step support
    {
        double tmp = endM;
        endM = startM;
        startM = tmp;
        assert(stepM < 0.);
        stepM = -stepM;
    }

    Math::Vec3 prevPosition;
    for (long stepIdx = 0; stepIdx <= numSteps; ++stepIdx)
    {
        double srcDistance = startM + stepIdx * stepM;
        std::optional<Math::Mat4> matrix = calcFunc(srcDistance);
        if (!matrix)
        {
            clear();
            return m_length;
        }

        const Math::Vec3 position = matrix->getTranslate();
        const double addLength = stepIdx ? (position - prevPosition).length() : 0.0;
        prevPosition = position;

        appendSegment(*matrix, addLength);
    }

    m_pointsTree.buildIndex();
    return m_length;
}

double
TransformSpline::build(const BezierSpline& first, const BezierSpline& second,
    const std::function<double(double)>& k, double step, double firstStartM, double secondStartM,
    std::optional<double> maxLengthM, bool keepLength)
{
    m_length = 0.0;
    m_points.clear();
    m_segments.clear();

    if (!maxLengthM)
    {
        maxLengthM = std::min(first.arcLength() - firstStartM, second.arcLength() - secondStartM);
    }
    const long numSteps = std::lround(*maxLengthM / step);
    if (numSteps == 0)
    {
        return 0.0;
    }
    m_points.reserve(numSteps);
    const double invNumSteps = 1.0 / numSteps;
    step = *maxLengthM * invNumSteps;

    double firstStep = step;
    double secondStep = step;
    if (keepLength)
    {
        firstStep = (first.arcLength() - firstStartM) * invNumSteps;
        secondStep = (second.arcLength() - secondStartM) * invNumSteps;
    }

    Math::Vec3 prevPosition;
    for (long stepIdx = 0; stepIdx <= numSteps; ++stepIdx)
    {
        const double firstOffset = stepIdx * firstStep;
        const std::optional<Math::Mat4> firstMatrix = first.calcSplineMatrixAtDistance(
            std::clamp(firstStartM + firstOffset, 0.0, first.arcLength()));
        const double secondOffset = stepIdx * secondStep;
        const std::optional<Math::Mat4> secondMatrix = second.calcSplineMatrixAtDistance(
            std::clamp(secondStartM + secondOffset, 0.0, second.arcLength()));
        if (!firstMatrix || !secondMatrix)
        {
            clear();
            return m_length;
        }

        const double offset = double(stepIdx) * invNumSteps;
        const Math::Mat4 matrix = Math::lerp(*firstMatrix, *secondMatrix, k(offset));

        const Math::Vec3 position = matrix.getTranslate();
        const double addLength = stepIdx ? (position - prevPosition).length() : 0.0;
        prevPosition = position;

        appendSegment(matrix, addLength);
    }

    m_pointsTree.buildIndex();
    return m_length;
}

void TransformSpline::clear()
{
    m_length = 0.0;
    m_segments.clear();
    m_points.clear();
}

std::optional<Math::Mat4>
TransformSpline::getTransformAtDistance(double distance) const
{
    auto segmentIt = m_segments.find(distance);
    if (segmentIt == m_segments.end())
    {
        return std::nullopt;
    }
    const boost::icl::continuous_interval<double, std::less> interval = segmentIt->first;
    const std::shared_ptr<Math::Mat4> nextTransform = segmentIt->second;
    if (--segmentIt == m_segments.end())
    {
        return *nextTransform;
    }
    const std::shared_ptr<Math::Mat4> prevTransform = segmentIt->second;

    const double k = (distance - interval.lower()) / (interval.upper() - interval.lower());
    return Math::lerp(*prevTransform, *nextTransform, k);
}

std::optional<TransformSpline::Point>
TransformSpline::findClosestPoint(const glm::dvec3& position) const
{
    size_t pointIndex = 0; double distSqr = 0.0;
    double queryPt[] = { position.x, position.y, position.z };
    if (!m_pointsTree.knnSearch(queryPt, 1, &pointIndex, &distSqr))
    {
        return std::nullopt;
    }
    const auto& point = m_points[pointIndex];

    auto segmentIt = m_segments.find(point.distanceOffset);
    if (segmentIt == m_segments.end())
    {
        return std::nullopt;
    }
    boost::icl::continuous_interval<double, std::less> interval = segmentIt->first;
    std::shared_ptr<Math::Mat4> transform = segmentIt->second;

    std::shared_ptr<Math::Mat4> prevTransform = transform;

    auto prevIt = segmentIt;
    if (--prevIt != m_segments.end())
    {
        prevTransform = prevIt->second;
    }

    auto nextIt = segmentIt;
    if (++nextIt != m_segments.end())
    {
        boost::icl::continuous_interval<double, std::less> nextInterval = nextIt->first;
        std::shared_ptr<Math::Mat4> nextTransform = nextIt->second;

        const Math::Vec3 dstPoint = fromGlm(position);
        if (prevTransform == transform ||
            (nextTransform->getTranslate() - dstPoint).length2() <
                (prevTransform->getTranslate() - dstPoint).length2())
        {
            prevTransform = transform;
            transform = nextTransform;
            interval = nextInterval;
        }
    }

    if (prevTransform == transform)
    {
        return TransformSpline::Point{point.distanceOffset, point.transform};
    }

    const double offset = nearestPointOffsetOnSegment(toGlm(prevTransform->getTranslate()),
        toGlm(transform->getTranslate()), position);

    Point result;
    result.distanceOffset =
        interval.lower() + (interval.upper() - interval.lower()) * offset;
    result.transform = Math::lerp(*prevTransform, *transform, offset);
    return result;
}

std::optional<TransformSpline::NeighbourPoints>
TransformSpline::findClosestPoints(const glm::dvec3& from, const glm::dvec3& to, double stepM) const
{
    std::optional<TransformSpline::NeighbourPoints> result;

    glm::dvec3 dir = to - from;
    const double length = glm::length(dir);
    dir = glm::normalize(dir);

    const int numDistanceSteps = std::lround(length / stepM);
    const double stepLength = length / numDistanceSteps;
    for (int stepIndex = 0; stepIndex < numDistanceSteps; ++stepIndex)
    {
        const double offset = stepLength * stepIndex;
        const glm::dvec3 position = from + dir * offset;

        size_t pointIndex = 0; double distSqr = 0.0;
        double queryPt[] = { position.x, position.y, position.z };
        if (m_pointsTree.knnSearch(queryPt, 1, &pointIndex, &distSqr))
        {
            if (!result || (result->gap > distSqr))
            {
                if (!result)
                {
                    result.emplace();
                }
                result->gap = distSqr;
                result->first = m_points[pointIndex];
                result->second.distanceOffset = offset;
                result->second.transform.setTranslate(fromGlm(position));
            }
        }
    }

    if (!result)
    {
        return std::nullopt;
    }

    auto segmentIt = m_segments.find(result->first.distanceOffset);
    if (segmentIt == m_segments.end())
    {
        return std::nullopt;
    }
    boost::icl::continuous_interval<double, std::less> interval = segmentIt->first;
    std::shared_ptr<Math::Mat4> transform = segmentIt->second;

    auto prevIt = segmentIt;
    if (--prevIt == m_segments.end())
    {
        result->gap = sqrt(result->gap);
        return result;
    }
    auto prevTransform = prevIt->second;

    auto nextIt = segmentIt;
    if (++nextIt != m_segments.end())
    {
        boost::icl::continuous_interval<double, std::less> nextInterval = nextIt->first;
        std::shared_ptr<Math::Mat4> nextTransform = nextIt->second;

        const Math::Vec3 secondPoint = result->second.transform.getTranslate();
        if ((nextTransform->getTranslate() - secondPoint).length2() <
            (prevTransform->getTranslate() - secondPoint).length2())
        {
            prevTransform = transform;
            transform = nextTransform;
            interval = nextInterval;
        }
    }

    const NearestPointsOffsets offsets = nearestPointsOffsetsBetweenTwoSegments(
        toGlm(prevTransform->getTranslate()), toGlm(transform->getTranslate()), from, to);

    result->first.distanceOffset =
        interval.lower() + (interval.upper() - interval.lower()) *
        offsets.segmentAOffset;
    result->first.transform = Math::lerp(*prevTransform, *transform, offsets.segmentAOffset);

    result->second.distanceOffset = offsets.segmentBOffset;
    result->second.transform = transformFromPositionAndTangent(
        fromGlm(from + dir * length * offsets.segmentBOffset), fromGlm(dir));

    result->gap =
        (result->second.transform.getTranslate() -
        result->first.transform.getTranslate()).length();

    return result;
}

std::optional<TransformSpline::NeighbourPoints>
TransformSpline::findClosestPoints(const TransformSpline& other) const
{
    std::optional<double> leastDistSqr;
    int leastDistPointIdx = -1;
    int leastDistOtherPointIdx = -1;

    for (int otherPointIdx = 0; otherPointIdx < other.m_points.size(); ++otherPointIdx)
    {
        const auto& otherPoint = other.m_points[otherPointIdx];
        const Math::dvec3 position = otherPoint.transform.getTranslate();

        size_t pointIndex = 0;
        double distSqr = 0.0;
        double queryPt[] = {position.x, position.y, position.z};
        if (m_pointsTree.knnSearch(queryPt, 1, &pointIndex, &distSqr))
        {
            if (!leastDistSqr || (leastDistSqr > distSqr))
            {
                leastDistSqr = distSqr;
                leastDistPointIdx = pointIndex;
                leastDistOtherPointIdx = otherPointIdx;
            }
        }
    }

    if (!leastDistSqr)
    {
        return std::nullopt;
    }

    auto getPossibleNearestSegments = [](const SegmentsIntervalMap& segments, double distance) {
        auto segmentIt = segments.find(distance);
        if (segmentIt == segments.end())
        {
            return std::vector<SegmentsIntervalMap::const_iterator>{};
        }

        std::vector<SegmentsIntervalMap::const_iterator> results;
        results.push_back(segmentIt);

        auto prevIt = segmentIt;
        if (--prevIt != segments.end())
        {
            results.insert(results.begin(), prevIt);
        }

        auto nextIt = segmentIt;
        if (++nextIt != segments.end())
        {
            results.push_back(nextIt);
        }

        return results;
    };

    TransformSpline::NeighbourPoints result{0.0, m_points[leastDistPointIdx],
        other.m_points[leastDistOtherPointIdx]};

    auto checkSegments = getPossibleNearestSegments(m_segments, result.first.distanceOffset);
    auto otherSegments = getPossibleNearestSegments(other.m_segments, result.second.distanceOffset);
    if (checkSegments.size() < 2 || otherSegments.size() < 2)
    {
        result.gap = sqrt(*leastDistSqr);
        return result;
    }

    for (int segmentIdx = 1; segmentIdx < checkSegments.size(); ++segmentIdx)
    {
        const auto& interval = checkSegments[segmentIdx]->first;
        const auto& prevTransform = *checkSegments[segmentIdx - 1]->second;
        const auto& transform = *checkSegments[segmentIdx]->second;

        for (int otherSegmentIdx = 1; otherSegmentIdx < otherSegments.size();
                ++otherSegmentIdx)
        {
            const auto& otherInterval = otherSegments[otherSegmentIdx]->first;
            const auto& otherPrevTransform = *otherSegments[otherSegmentIdx - 1]->second;
            const auto& otherTransform = *otherSegments[otherSegmentIdx]->second;

            const NearestPointsOffsets offsets = nearestPointsOffsetsBetweenTwoSegments(
                toGlm(prevTransform.getTranslate()), toGlm(transform.getTranslate()),
                toGlm(otherPrevTransform.getTranslate()),
                toGlm(otherTransform.getTranslate()));

            auto firstTransform =
                Math::lerp(prevTransform, transform, offsets.segmentAOffset);
            auto secondTransform =
                Math::lerp(otherPrevTransform, otherTransform, offsets.segmentBOffset);
            double distSqr =
                (firstTransform.getTranslate() - secondTransform.getTranslate()).length2();

            if (leastDistSqr > distSqr)
            {
                leastDistSqr = distSqr;

                result.first.distanceOffset = interval.lower() +
                    (interval.upper() - interval.lower()) * offsets.segmentAOffset;
                result.first.transform = firstTransform;

                result.second.distanceOffset = otherInterval.lower() +
                    (otherInterval.upper() - otherInterval.lower()) *
                        offsets.segmentBOffset;
                result.second.transform = secondTransform;
            }
        }
    }

    result.gap = sqrt(*leastDistSqr);
    return result;
}

std::optional<TransformSpline::NeighbourPoints>
TransformSpline::findPointsWithDistance(const BezierSpline& second, double findDistance, double stepM,
    double startM, std::optional<double> endM) const
{
    // TODO: accuracy is not optimized here yet - no proper way found. If will be done (and will fit),
    // fixed step in 'build' functions should be replaced with adaptive spline partitioning.

    std::optional<TransformSpline::NeighbourPoints> result;
    double findDistanceSqr = findDistance * findDistance;

    double secondLen = second.arcLength();
    if (!endM || (endM > secondLen))
    {
        endM = secondLen;
    }

    const double useLength = *endM - startM;
    const int numDistanceSteps = std::abs(std::lround(useLength / stepM));
    stepM = useLength / numDistanceSteps;

    for (int stepIndex = 0; stepIndex < numDistanceSteps; ++stepIndex)
    {
        const double offset = startM + stepM * stepIndex;
        const std::optional<glm::dvec3> position = second.pointAtDistance(offset);
        if (!position)
        {
            return std::nullopt;
        }

        size_t pointIndex = 0; double distSqr = 0.0;
        double queryPt[] = { position->x, position->y, position->z };
        if (m_pointsTree.knnSearch(queryPt, 1, &pointIndex, &distSqr))
        {
            const auto& point = m_points[pointIndex];
            if (!result ||
                std::abs(result->gap - findDistanceSqr) >
                std::abs(distSqr - findDistanceSqr))
            {
                if (!result)
                {
                    result.emplace();
                }
                result->gap = distSqr;

                result->first = point;

                result->second.distanceOffset = offset;
            }
        }
    }

    if (result)
    {
        if (auto secondTransform = second.calcSplineMatrixAtDistance(result->second.distanceOffset))
        {
            result->gap = sqrt(result->gap);
            result->second.transform = *secondTransform;
        }
        else
        {
            return std::nullopt;
        }
    }
    return result;
}

std::optional<TransformSpline::NeighbourPoints>
TransformSpline::findPointsAcrossWithDistance(const TransformSpline& from,
    const TransformSpline& to, double findDistance, double stepM, double startM,
    std::optional<double> endM) const
{
    std::optional<TransformSpline::NeighbourPoints> result;
    double findDistanceSqr = findDistance * findDistance;

    double thisLen = arcLength();
    if (!endM || (endM > thisLen))
    {
        endM = thisLen;
    }

    const double useLength = *endM - startM;
    const int numDistanceSteps = std::abs(std::lround(useLength / stepM));
    stepM = useLength / numDistanceSteps;

    for (int stepIndex = 0; stepIndex < numDistanceSteps; ++stepIndex)
    {
        const double offset = startM + stepM * stepIndex;
        const std::optional<Math::Mat4> transform = getTransformAtDistance(offset);
        if (!transform)
        {
            return std::nullopt;
        }

        auto fromPoint = toGlm(*transform * Math::Vec3(0, findDistance, 0));
        auto toPoint = toGlm(*transform * Math::Vec3(0, -findDistance, 0));

        double absStepM = std::abs(stepM);
        auto fromClosest = from.findClosestPoints(fromPoint, toPoint, absStepM);
        auto toClosest = to.findClosestPoints(fromPoint, toPoint, absStepM);

        if (!fromClosest || !toClosest)
        {
            continue;
        }

        double distSqr = Math::length2(fromClosest->first.transform.getTranslate() -
            toClosest->first.transform.getTranslate());

        if (!result ||
            std::abs(result->gap - findDistanceSqr) > std::abs(distSqr - findDistanceSqr))
        {
            if (!result)
            {
                result.emplace();
            }
            result->gap = distSqr;

            result->first = fromClosest->first;
            result->second = toClosest->first;
        }
    }

    if (result)
    {
        result->gap = sqrt(result->gap);
    }
    return result;
}

void TransformSpline::setTestVectorFunc(const TransformSpline::TestVectorFunc& func)
{
#ifdef _DEBUG
    m_testVector = func;
#endif
}

void TransformSpline::testVector(const Math::Vec3& from, const Math::Vec3& to,
    const Math::vec4& color) const
{
#ifdef _DEBUG
    if (m_testVector)
    {
        m_testVector(from, to, color);
    }
#endif
}

double
TransformSpline::appendSegment(const Math::Mat4& transfom, double length)
{
    if (m_segments.empty() && length <= 0.)
    {
        m_segments.insert({boost::icl::interval<double>::closed(m_length, m_length + length),
            std::make_shared<Math::Mat4>(transfom)});
    }
    else
    {
        assert(length > 0.);
        m_segments.insert({boost::icl::interval<double>::left_open(m_length, m_length + length),
            std::make_shared<Math::Mat4>(transfom)});
    }
    m_points.emplace_back(Point{m_length + length, transfom});
    return m_length += length;
}
