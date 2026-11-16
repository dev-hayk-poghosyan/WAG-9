#include "GenerateEnoughSplinePoints.h"

using namespace Unigine;

/**
 * Code below is provided by Unigine tech support here:
 * https://developer.unigine.com/ru/personal/support-ticket/6201
 */

const float tangentialOffsetThresholdSquared = 0.01f * 0.01f;
const float segmentPartParametricThreshold = 1.0f / 20.0f;

using SplinePointDataCallbackFunc = std::function<void(float, const Math::Vec3&, const Math::Scalar&)>;

void
generateEnoughSplinePoints(const std::function<Math::Vec3(double)>& calcPoint,
    const SplinePointDataCallbackFunc& addData, float ta, float tb)
{
    VectorStack<float> stack;
    stack.append(tb);
    stack.append(ta);

    Math::Vec3 p1;
    Math::Vec3 p0;
    while (stack.size())
    {
        float t0 = stack.takeLast();
        float t1 = stack.takeLast();

        p0 = calcPoint(t0);
        p1 = calcPoint(t1);

        float tm1 = (2.0f * t0 + t1) / 3.0f;
        float tm2 = (t0 + 2.0f * t1) / 3.0f;

        Math::Vec3 pm0(calcPoint(tm1));
        Math::Vec3 pm1(calcPoint(tm2));

        Math::Vec3 v = p1 - p0;
        Math::Scalar lengthSquared = length2(v);
        Math::Scalar maxTangentialOffsetSquared =
            std::max(length2(cross(pm0 - p0, v)) / lengthSquared,
                length2(cross(pm1 - p0, v)) / lengthSquared);

        if (((t1 - t0) < segmentPartParametricThreshold) ||
            (maxTangentialOffsetSquared < tangentialOffsetThresholdSquared))
        {
            addData(t1, p1, maxTangentialOffsetSquared);
            continue;
        }

        float tm = (t0 + t1) * 0.5f;
        stack.append(t1);
        stack.append(tm);
        stack.append(tm);
        stack.append(t0);
    }
}

void generateEnoughSplinePoints(const std::function<Math::Vec3(double)>& calcPoint,
    Vector<Math::Vec3>& points,
    std::vector<float>& tParams,
    float ta,
    float tb)
{
    auto addData = [&tParams, &points](float t1, const Math::Vec3& p1, const Math::Scalar& _)
    {
        tParams.push_back(t1);
        points.append(p1);
    };

    generateEnoughSplinePoints(calcPoint, addData, ta, tb);
}

void
generateEnoughSplinePoints(const std::function<Math::Vec3(double)>& calcPoint,
    Vector<Math::Vec3>& points,
    std::vector<float>& tParams,
    std::vector<double>& tangentialOffsetsSquared,
    float ta, float tb)
{
    auto addData = [&tParams, &points, &tangentialOffsetsSquared](float t1, const Math::Vec3& p1,
                       const Math::Scalar& tangentialOffsetSquared)
    {
        tangentialOffsetsSquared.push_back(tangentialOffsetSquared);
        tParams.push_back(t1);
        points.append(p1);
    };

    generateEnoughSplinePoints(calcPoint, addData, ta, tb);
}
