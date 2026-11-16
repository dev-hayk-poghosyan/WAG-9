#include "GenerateMeshFromSplineList.h"

#include "neokon-shared/BezierArcLengthCalculator.h"
#include "neokon-shared/BezierSegment.h"
#include "neokon-shared/GenerateEnoughSplinePoints.h"
#include "neokon-shared/GlmUnigineInterop.h"

#include <sstream>
#include <optional>

using namespace Unigine;

namespace
{

/**
 * Выпрямить сплайны. Функция проверяет чтобы направления всех сегментов пути
 * были сонаправлены общему вектору пути. Если сплайн ориентирован в противоположную
 * сторону, функция его переворачивает.
 */
void straightenSplineList(std::vector<BezierSegment>& inoutSplineList)
{
    if (inoutSplineList.size() < 2)
    {
        return;
    }

    BezierSegment const& firstSegment  = inoutSplineList.front();
    BezierSegment const& lastSegment = inoutSplineList.back();
    Math::Vec3 const pathDirection = fromGlm(lastSegment.p3()) - fromGlm(firstSegment.p0());

    for(int i = 0; i < inoutSplineList.size(); i++) 
    {
        BezierSegment const& segment = inoutSplineList[i];
        Math::Vec3 const segmentDirection = fromGlm(segment.p3())- fromGlm(segment.p0());
        double dot = Math::dot(pathDirection, segmentDirection);
        if (dot < 0.0)
        {
            inoutSplineList[i] = BezierSegment(segment.p3(), segment.p2(),
                                               segment.p1(), segment.p0());
        }
    }
}

std::vector<float>
generateSplineControlPointsAdaptive(const BezierSegment& segment, float tBegin, float tEnd)
{
    auto calcSplinePoint = [&segment](double param)
    {
        glm::dvec3 glmPosition = segment.eval(param);
        Math::Vec3 position = fromGlm(glmPosition);
        return position;
    };

    std::vector<float> points;

    points.emplace_back(tBegin);
    if (tBegin < 0.5f)
    {
        generateEnoughSplinePoints(calcSplinePoint, Vector<Math::Vec3>(), points,
            tBegin, std::min(tEnd, 0.5f));
    }
    if (tEnd > 0.5f)
    {
        generateEnoughSplinePoints(calcSplinePoint, Vector<Math::Vec3>(), points,
            std::max(tBegin, 0.5f), tEnd);
    }

    return points;
};

std::vector<float>
generateSplineControlPointsFixedStep(const BezierSegment& segment, float tBegin, float tEnd, double stepM)
{
    std::vector<float> points;
    points.push_back(tBegin);

    BezierArcLengthCalculator lengthCalculator(segment);

    double fullLength = lengthCalculator.arcLengthFromTo(tBegin, tEnd);
    long numSegments = std::lround(fullLength / stepM);
    if (numSegments < 1)
    {
        numSegments = 1;
    }

    double beginLength = (tBegin > 0.0f) ? lengthCalculator.arcLengthFromTo(0.0f, tBegin) : 0.0;
    for (long segmentIdx = 0; segmentIdx < numSegments; ++segmentIdx)
    {
        if (segmentIdx == numSegments - 1)
        {
            points.push_back(tEnd);
            break;
        }
        points.push_back(lengthCalculator.tForDistance(beginLength + stepM * segmentIdx));
    }

    return points;
};

}//Anonymous namespace

GenerateMeshFromSplineOutput
generateMeshFromSpline(const GenerateMeshFromSplineArgs& args)
{
    assert(args.cropLengthBegin <= args.cropLengthEnd);

    GenerateMeshFromSplineOutput output;

    struct SplitPoint
    {
        SplitPoint(BezierSegment const& segment)
            : segment(segment)
        {
        }

        const BezierSegment& segment;
        double tParam{ 0.0 };
        double distance{ 0.0 };
    };

    std::list<SplitPoint> splitPoints;

    std::vector<BezierSegment> localSplineList = args.splineList;
    straightenSplineList(localSplineList);

    auto calcSplitPoints = [&splitPoints, &localSplineList, &args]()
    {
        double offset = 0.0;
        for (BezierSegment const& segment : localSplineList)
        {
            BezierArcLengthCalculator lengthCalculator(segment);
            double segmentLength = lengthCalculator.arcLengthFromTo(0.0, 1.0);

            float tBegin = 0.0f;
            float tEnd = 1.0f;
            if (offset < args.cropLengthBegin)
            {
                if (offset + segmentLength > args.cropLengthBegin)
                {
                    tBegin = lengthCalculator.tForDistance(args.cropLengthBegin - offset);
                }
                else
                {
                    offset += segmentLength;
                    continue;
                }
            }
            if (args.cropLengthEnd > 0.0)
            {
                if (offset < args.cropLengthEnd)
                {
                    if (offset + segmentLength > args.cropLengthEnd)
                    {
                        tEnd = lengthCalculator.tForDistance(args.cropLengthEnd - offset);
                    }
                }
                else
                {
                    break;
                }
            }

            std::vector<float> splineControlPoints;
            if (args.adaptiveDividing)
            {
                splineControlPoints = generateSplineControlPointsAdaptive(segment, tBegin, tEnd);
            }
            else
            {
                splineControlPoints = generateSplineControlPointsFixedStep(segment, tBegin, tEnd,
                    1.0 / args.numSegmentsPerUnit);
            }
            for (const auto& point : splineControlPoints)
            {
                auto& splitPoint = splitPoints.emplace_back(segment);
                splitPoint.tParam = point;
                splitPoint.distance = offset + lengthCalculator.arcLengthFromTo(0.0, point);
            }

            offset += segmentLength;
        }

        return offset;
    };

    double pathLength = calcSplitPoints();
    if (splitPoints.empty())
    {
        Unigine::Log::error("generateMeshFromSpline failed, splitPoints.empty()");
        return output;
    }

    int currentProfileScalingKeyIdx = 0;
    auto calcSplitPointTransform =
        [&currentProfileScalingKeyIdx, &args, &pathLength](const SplitPoint& splitPoint)
    {
        auto calcSplinePointMatrix = [](BezierSegment const& segment, double param)
        {
            glm::dvec3 glmPosition = segment.eval(param);
            glm::dvec3 glmTangent = segment.unitTangentAt(param);

            Math::Vec3 position = fromGlm(glmPosition);
            Math::Vec3 tangent = fromGlm(glmTangent);
            tangent.z = 0.0;
            tangent.normalize();

            Math::Vec3 upVector = Math::Vec3::UP;
            Math::Vec3 normal = Math::cross(upVector, tangent);

            return Math::Mat4(tangent, normal, upVector, position);
        };

        auto findNextProfileScalingKey = [&args, &pathLength](double pathOffset, int& startIdx)
            -> std::optional<int>
        {
            for (int i = startIdx; i < args.profileScaleKeys.size(); i++)
            {
                ProfileScaleKey const& key = args.profileScaleKeys[i];
                double absolutePathOffest = key.normalizedPathOffset * pathLength;
                if (pathOffset < absolutePathOffest)
                {
                    startIdx = i;
                    return i;
                }
            }
            return std::nullopt;
        };

        Math::Mat4 transform = calcSplinePointMatrix(splitPoint.segment, splitPoint.tParam);

        if (auto scalingKeyIdxOpt = findNextProfileScalingKey(
            splitPoint.distance, currentProfileScalingKeyIdx))
        {
            int startKeyIdx = *scalingKeyIdxOpt - 1;
            int endKeyIdx = *scalingKeyIdxOpt;
            ProfileScaleKey const& startKey = args.profileScaleKeys[startKeyIdx];
            ProfileScaleKey const& endKey = args.profileScaleKeys[endKeyIdx];
            double normalizedPathOffset = splitPoint.distance / pathLength;
            double divisionUp = normalizedPathOffset - startKey.normalizedPathOffset;
            double divisionDown = endKey.normalizedPathOffset - startKey.normalizedPathOffset;
            double lerpKey = divisionUp / divisionDown;
            Math::Vec2 scalingValues = Math::lerp(startKey.profileAxisScaleKey,
                endKey.profileAxisScaleKey, lerpKey);
            Math::Mat4 scalingMat = Math::Mat4_identity;
            scalingMat.m11 = scalingValues.x;
            scalingMat.m22 = scalingValues.y;

            //transform = scalingMat * transform;
            transform = transform * scalingMat;
        }

        return transform;
    };

    int surfaceIdx = args.surfaceIndex;
    int cIndexOffset = 0;
    int tIndexOffset = 0;
    if (surfaceIdx == -1)
    {
        surfaceIdx = args.dstMesh->addSurface(args.surfaceName.c_str());
    }
    else
    {
        cIndexOffset = args.dstMesh->getNumVertex(surfaceIdx);
        tIndexOffset = args.dstMesh->getNumTexCoords0(surfaceIdx);
    }

    const int numProfilePoints = args.profilePointPositions.size();

    auto getPos = [&args, &numProfilePoints](int i)
    {
        i = i % numProfilePoints;
        if (i < 0)
        {
            i = numProfilePoints + i;
        }
        if (args.profileMirrorY)
        {
            int useIndex = numProfilePoints - 1 - i;
            return args.profilePointPositions[useIndex] * Math::Vec3(1., -1., 1.) +
                args.profileOffset;
        }
        return args.profilePointPositions[i] + args.profileOffset;
    };

    auto getV = [&args, &numProfilePoints](int i)
    {
        if (args.profileMirrorY)
        {
            int useIndex = numProfilePoints - 1 - i;
            return args.profilePointUVCoords[useIndex].y;
        }
        return args.profilePointUVCoords[i].y;
    };

    auto getNormal = [&args, &numProfilePoints](int i)
    {
        if (args.profileMirrorY)
        {
            int useIndex = numProfilePoints - 1 - i;
            return args.profilePointNormals[useIndex] * Math::vec3(1, -1, 1);
        }
        return args.profilePointNormals[i];
    };

    auto isTangentFlipped = [](const Math::quat& toCheck)
    {
        auto normal = toCheck.getNormal();
        auto tangent = toCheck.getTangent();
        auto binormal1 = cross(normal, tangent);
        auto binormal2 = toCheck.getBinormal();
        return (dot(binormal1, binormal2) < 0.0f);
    };

    auto calcTangent = [&getNormal, &getV, numProfilePoints, &isTangentFlipped](int i, 
        const Math::Mat4& matrix)
    {
        Math::vec3 normal = getNormal(i);
        Math::vec3 tangent = Math::vec3_right;
        auto binormal = Math::cross(normal, tangent);

        Math::quat localTangent(tangent, binormal, normal);
        localTangent.normalize();

        auto result = matrix.getRotate() * localTangent;

        float vdiff = 0.0f;
        if (i)
        {
            vdiff = getV(i) - getV(i - 1);
        }
        if (i < numProfilePoints - 1)
        {
            vdiff += getV(i + 1) - getV(i);
        }

        if (std::abs(result.w) < 0.001f)
        {
            result.w = std::copysign(0.001f, result.w);
        }

        if (((vdiff < 0.0f) && isTangentFlipped(result)) || 
            ((vdiff >= 0.0f) && !isTangentFlipped(result)))
        {
            result = result * -1.0f;
        }

        return result;
    };

    auto generateCap = [&args, &getPos, surfaceIdx, numProfilePoints](const Math::Mat4& matrix,
        const Math::vec3& normal, bool backFace)
    {
        if (args.profileCapTris.empty())
        {
            return;
        }

        auto calcCapTangent = [](const Math::vec3& normal)
        {
            Math::vec3 tangent = Math::vec3_down;
            Math::mat3 rotation(tangent, Math::cross(tangent, normal), normal);
            Math::quat result(rotation);
            if (result.w < 0.001f)
            {
                result.w = 0.001f;
            }
            return result;
        };

        Math::vec2 texCoord0(0, 0);
        int cIndex = args.dstMesh->getNumVertex(surfaceIdx);
        int tIndex = args.dstMesh->getNumTexCoords0(surfaceIdx);
        for (int i = 0; i < numProfilePoints; i++)
        {
            Math::Vec3 localPos = getPos(i);
            Math::Vec3 position = matrix * localPos;

            args.dstMesh->addVertex(Math::vec3(position), surfaceIdx);
            args.dstMesh->addTexCoord0(texCoord0, surfaceIdx);
            args.dstMesh->addNormal(matrix.getRotate() * normal, surfaceIdx);
            args.dstMesh->addTangent(Math::quat(matrix) * calcCapTangent(normal), surfaceIdx);
        }
        for (const auto& tri : args.profileCapTris)
        {
            args.dstMesh->addCIndex(cIndex + tri[0], surfaceIdx);
            args.dstMesh->addCIndex(cIndex + tri[backFace ? 2 : 1], surfaceIdx);
            args.dstMesh->addCIndex(cIndex + tri[backFace ? 1 : 2], surfaceIdx);

            args.dstMesh->addTIndex(tIndex + tri[0], surfaceIdx);
            args.dstMesh->addTIndex(tIndex + tri[backFace ? 2 : 1], surfaceIdx);
            args.dstMesh->addTIndex(tIndex + tri[backFace ? 1 : 2], surfaceIdx);
        }
    };

    for (const auto& splitPoint : splitPoints)
    {
        double uCoord = args.uCoordStepPerUnit * (splitPoint.distance + args.lengthOffsetForUV);
        auto transform = calcSplitPointTransform(splitPoint);
        for (int i = 0; i < numProfilePoints; i++)
        {
            const Math::Vec3 localPos = getPos(i);
            const Math::vec2 texCoord0(uCoord, getV(i));
            const Math::vec3 localNormal = getNormal(i);

            Math::Vec3 position = transform * localPos;

            args.dstMesh->addVertex(Math::vec3(position), surfaceIdx);
            args.dstMesh->addTexCoord0(texCoord0, surfaceIdx);
            args.dstMesh->addNormal(transform.getRotate() * localNormal, surfaceIdx);
            args.dstMesh->addTangent(calcTangent(i, transform), surfaceIdx);
        }
    }//for (const auto& splitPoint : splitPoints)

    if (args.preCap)
    {
        auto transform = calcSplitPointTransform(splitPoints.front());
        generateCap(transform, Math::vec3_right, true);
    }

    //indexes
    const int numSegments = splitPoints.size() - 1;
    const int numQuadsPerSegment = numProfilePoints - 1;
    const int vertexStep = numProfilePoints;
    for (int i = 0; i < numSegments; i++)
    {
        for (int j = 0; j < numQuadsPerSegment; j++)
        {
            int iv0 = (i * vertexStep) + j;
            int iv1 = (i + 1) * vertexStep + j;
            int iv2 = (i + 1) * vertexStep + j + 1;
            int iv3 = (i * vertexStep) + j + 1;

            args.dstMesh->addCIndex(cIndexOffset + iv2, surfaceIdx);
            args.dstMesh->addCIndex(cIndexOffset + iv1, surfaceIdx);
            args.dstMesh->addCIndex(cIndexOffset + iv0, surfaceIdx);

            args.dstMesh->addCIndex(cIndexOffset + iv3, surfaceIdx);
            args.dstMesh->addCIndex(cIndexOffset + iv2, surfaceIdx);
            args.dstMesh->addCIndex(cIndexOffset + iv0, surfaceIdx);

            args.dstMesh->addTIndex(tIndexOffset + iv2, surfaceIdx);
            args.dstMesh->addTIndex(tIndexOffset + iv1, surfaceIdx);
            args.dstMesh->addTIndex(tIndexOffset + iv0, surfaceIdx);

            args.dstMesh->addTIndex(tIndexOffset + iv3, surfaceIdx);
            args.dstMesh->addTIndex(tIndexOffset + iv2, surfaceIdx);
            args.dstMesh->addTIndex(tIndexOffset + iv0, surfaceIdx);
        }//for (int j = 0; j < numQuadsPerSegment; j++)
    }//for (int i = 0; i < numSegments; i++)

    if (args.postCap)
    {
        auto transform = calcSplitPointTransform(splitPoints.back());
        generateCap(transform, Math::vec3_left, false);
    }

    return output;
}
