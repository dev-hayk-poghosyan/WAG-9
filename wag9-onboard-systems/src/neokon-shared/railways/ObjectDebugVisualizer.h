#pragma once

#include <UnigineMathLib.h>
#include <UnigineMesh.h>
#include <UnigineString.h>

#include "neokon-shared/BezierSpline.h"
#include "neokon-shared/TransformSpline.h"

#include <vector>
#include <memory>

/**
 * Class for storage and deffered visualization of geometry primitives, intended to be used by
 * mesh generators for debugging of generation problems.
 */
class ObjectDebugVisualizer
{
public:
    /**
     * Add point to be displayed later. 
     */
    void addPoint(const Unigine::Math::Vec3& point, const Unigine::Math::vec4& color =
        Unigine::Math::vec4::RED);

    /**
     * Add vector to be displayed later.
     */
    void addVector(const Unigine::Math::Vec3& from, const Unigine::Math::Vec3& to,
        const Unigine::Math::vec4& color = Unigine::Math::vec4::RED, float arrowSize = 0.1f);

    /**
     * Add transform to be displayed as 3 axis vectors with translation later.
     */
    void addTransform(const Unigine::Math::Mat4& transform);

    /**
     * Add spline to be displayed later.
     */
    void addSpline(const BezierSpline& spline,
        const Unigine::Math::vec4& color = Unigine::Math::vec4::RED);

    /**
     * Add spline to be displayed later.
     */
    void addSpline(const TransformSpline& spline,
        const Unigine::Math::vec4& color = Unigine::Math::vec4::RED);

    /**
     * Add mesh tangents.
     */
    void addMeshTangents(const Unigine::MeshPtr& mesh);

    /**
     * Add text message
     */
    void addMesage(const Unigine::String& text, const Unigine::Math::Vec3& position = {},
        int fontSize = -1, const Unigine::Math::vec4& color = Unigine::Math::vec4::RED);

    /**
     * Visualize primitives at, applying 'transform' to them.
     */
    void visualize(const Unigine::Math::Mat4& transform) const;

private:
    struct Point
    {
        Unigine::Math::Vec3 position;
        Unigine::Math::vec4 color;
    };

    struct Vector
    {
        Unigine::Math::Vec3 fromPosition;
        Unigine::Math::Vec3 toPosition;
        Unigine::Math::vec4 color;
        float arrowSize{0.1f};
    };

    struct Message
    {
        Unigine::String text;
        Unigine::Math::Vec3 position;
        int fontSize{-1};
        Unigine::Math::vec4 color;
    };

    std::vector<Point> m_points;
    std::vector<Vector> m_vectors;
    std::vector<Message> m_messages;
};

using ObjectDebugVisualizerPtr = std::shared_ptr<ObjectDebugVisualizer>;
