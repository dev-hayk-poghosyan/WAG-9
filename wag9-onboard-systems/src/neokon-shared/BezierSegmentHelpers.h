#pragma once

#include "BezierSegment.h"

#include <UnigineMathLib.h>
#include <optional>

BezierSegment bezierSegmentByPositionsAndTangents(Unigine::Math::Vec3 const& startPointPosition,
    Unigine::Math::Vec3 const& endPointPosition, Unigine::Math::Vec3 const& startPointTangent,
    Unigine::Math::Vec3 const& endPointTangent);

std::optional<BezierSegment> bezierSegmentByThreePoints(Unigine::Math::Vec3 const& startPoint,
    Unigine::Math::Vec3 const& middlePoint, Unigine::Math::Vec3 const& endPoint);
