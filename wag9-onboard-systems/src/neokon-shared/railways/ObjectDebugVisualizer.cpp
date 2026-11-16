#include "ObjectDebugVisualizer.h"
#include <UnigineVisualizer.h>

#include "neokon-shared/GenerateEnoughSplinePoints.h"
#include "neokon-shared/GlmUnigineInterop.h"

using namespace Unigine;

void ObjectDebugVisualizer::addPoint(const Math::Vec3& point, const Math::vec4& color)
{
    auto& added = m_points.emplace_back();
    added.position = point;
    added.color = color;
}

void ObjectDebugVisualizer::addVector(const Math::Vec3& from, const Math::Vec3& to,
    const Math::vec4& color, float arrowSize)
{
    auto& added = m_vectors.emplace_back();
    added.arrowSize = arrowSize;
    added.fromPosition = from;
    added.toPosition = to;
    added.color = color;
}

void
ObjectDebugVisualizer::addTransform(const Unigine::Math::Mat4& transform)
{
    auto translate = transform.getTranslate();

    addVector(translate, translate + transform.getAxisX(), Math::vec4_red);
    addVector(translate, translate + transform.getAxisY(), Math::vec4_green);
    addVector(translate, translate + transform.getAxisZ(), Math::vec4_blue);
}

void
ObjectDebugVisualizer::addMeshTangents(const Unigine::MeshPtr& mesh)
{
    int numSurfaces = mesh->getNumSurfaces();
    for (int surfaceIdx = 0; surfaceIdx < numSurfaces; ++surfaceIdx)
    {
        int numTangents = mesh->getNumTangents(surfaceIdx);
        for (int tangentIdx = 0; tangentIdx < numTangents; ++tangentIdx)
        {
            Unigine::Math::Vec3 vertex(mesh->getVertex(tangentIdx, surfaceIdx));
            Unigine::Math::quat tangent = mesh->getTangent(tangentIdx, surfaceIdx);

            auto right = (tangent* Math::Vec3_right) * 0.1;
            auto up = (tangent * Math::Vec3_up) * 0.1;
            auto forward = (tangent * Math::Vec3_forward) * 0.1;

            addVector(vertex, vertex + forward, Math::vec4_green);
            addVector(vertex, vertex + right, Math::vec4_red);
            addVector(vertex, vertex + up, Math::vec4_blue);
        }
    }
}

void
ObjectDebugVisualizer::addMesage(const Unigine::String& text,
    const Unigine::Math::Vec3& position, int fontSize,
    const Unigine::Math::vec4& color)
{
    m_messages.emplace_back(Message{text, position, fontSize, color});
}

void
ObjectDebugVisualizer::addSpline(const BezierSpline& spline,
    const Unigine::Math::vec4& color)
{
    for (const auto& segmentPair : spline.getSegmentsMap())
    {
        auto calcSplinePoint = [&segmentPair](double param)
        {
            glm::dvec3 glmPosition = segmentPair.second->eval(param);
            Math::Vec3 position = fromGlm(glmPosition);
            return position;
        };

        Unigine::Vector<Math::Vec3> points;
        points.emplace_back(calcSplinePoint(0.0));
        generateEnoughSplinePoints(calcSplinePoint, points, std::vector<float>(), 0.0f, 0.5f);
        generateEnoughSplinePoints(calcSplinePoint, points, std::vector<float>(), 0.5f, 1.0f);

        for (int pointIdx = 1; pointIdx < points.size(); ++pointIdx)
        {
            const auto& prevPoint = points[pointIdx - 1];
            const auto& point = points[pointIdx];
            addVector(prevPoint, point, color, 0.01f);
        }
    }
}

void
ObjectDebugVisualizer::addSpline(const TransformSpline& spline,
    const Unigine::Math::vec4& color)
{
    const auto& points = spline.getPoints();
    for (int pointIdx = 1; pointIdx < points.size(); ++pointIdx)
    {
        const auto& prevPoint = points[pointIdx - 1];
        const auto& point = points[pointIdx];
        addVector(prevPoint.transform.getTranslate(), point.transform.getTranslate(), color, 0.01f);
    }
}

void ObjectDebugVisualizer::visualize(const Math::Mat4& transform) const
{
    for (const auto& point : m_points)
    {
        Visualizer::renderPoint3D(transform * point.position, /*size=*/0.05f, point.color,
            /*screen_space=*/false, /*duration=*/10000.0f, /*depth_test=*/false);
    }

    for (const auto& vector : m_vectors)
    {
        Visualizer::renderVector(transform * vector.fromPosition, transform * vector.toPosition,
            vector.color, vector.arrowSize, /*screen_space=*/false, /*duration=*/10000.0f,
            /*depth_test=*/false);
    }

    for (const auto& message : m_messages)
    {
        Visualizer::renderMessage3D(transform * message.position, Math::vec3(), message.text.get(),
            message.color, /*outline=*/0, /*font_size=*/-1, /*duration=*/10000.0f);
    }
}
