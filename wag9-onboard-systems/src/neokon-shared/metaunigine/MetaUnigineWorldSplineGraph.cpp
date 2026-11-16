#include "MetaUnigineWorldSplineGraph.h"

#include <UnigineString.h>
using namespace Unigine;

namespace MetaUnigine
{

// SplinePoint

SplinePoint::SplinePoint(const Math::Vec3& position)
    : m_position(position)
{
}

void SplinePoint::setPosition(const Math::Vec3& position)
{
    m_position = position;
}

Math::Vec3 SplinePoint::getPosition() const
{
    return m_position;
}

// SplineSegment

SplineSegment::SplineSegment(const SplinePointPtr& start_point, 
    const Math::vec3& start_tangent, const Math::vec3& start_up, const SplinePointPtr& end_point, 
    const Math::vec3& end_tangent, const Math::vec3& end_up)
    : m_start_point(start_point)
    , m_start_tangent(start_tangent)
    , m_start_up(start_up)
    , m_end_point(end_point)
    , m_end_tangent(end_tangent)
    , m_end_up(end_up)
{
}

void SplineSegment::setStartPoint(const SplinePointPtr& point)
{
    m_start_point = point;
}

SplinePointPtr SplineSegment::getStartPoint() const
{
    return m_start_point;
}

void SplineSegment::setEndPoint(const SplinePointPtr& point)
{
    m_end_point = point;
}

SplinePointPtr SplineSegment::getEndPoint() const
{
    return m_end_point;
}

void SplineSegment::setStartTangent(const Math::vec3& tangent)
{
    m_start_tangent = tangent;
}

Math::vec3 SplineSegment::getStartTangent() const
{
    return m_start_tangent;
}

void SplineSegment::setStartUp(const Math::vec3& up)
{
    m_start_up = up;
}

Math::vec3 SplineSegment::getStartUp() const
{
    return m_start_up;
}

void SplineSegment::setEndTangent(const Math::vec3& tangent)
{
    m_end_tangent = tangent;
}

Math::vec3 SplineSegment::getEndTangent() const
{
    return m_end_tangent;
}

void SplineSegment::setEndUp(const Math::vec3& up)
{
    m_end_up = up;
}

Math::vec3 SplineSegment::getEndUp() const
{
    return m_end_up;
}

// WorldSplineGraph

WorldSplineGraphPtr WorldSplineGraph::create()
{
    return std::make_shared<WorldSplineGraph>();
}

WorldSplineGraph::WorldSplineGraph()
{
    m_typeName = "WorldSplineGraph";
}

XmlPtr WorldSplineGraph::saveXml(const XmlPtr& xmlNode) const
{
    xmlNode->addChild("spline_name")->setData(m_splineGraphName.data());
    xmlNode->addChild("point_nodes");
    return Node::saveXml(xmlNode);
}

bool WorldSplineGraph::loadXml(const XmlPtr& xml)
{
    return Node::loadXml(xml);
}

SplinePointPtr WorldSplineGraph::createSplinePoint(const Math::Vec3& position)
{
    return m_points.emplace_back(std::make_shared<SplinePoint>(position));
}

SplineSegmentPtr WorldSplineGraph::createSplineSegment(const SplinePointPtr& start_point,
    const Math::vec3& start_tangent, const Math::vec3& start_up,
    const SplinePointPtr& end_point, const Math::vec3& end_tangent,
    const Math::vec3& end_up)
{
    return m_segments.emplace_back(std::make_shared<SplineSegment>(start_point, start_tangent,
        start_up, end_point, end_tangent, end_up));
}

template<typename VecType>
void jsonSaveVec(const JsonPtr& json, const VecType& vec)
{
    json->setArray();
    json->addChild(nullptr, vec.x);
    json->addChild(nullptr, vec.y);
    json->addChild(nullptr, vec.z);
}

bool WorldSplineGraph::save(const char* name)
{
    auto json = Json::create();

    auto jsonPoints = json->addChild("points");
    jsonPoints->setArray();
    for (const auto& point : m_points)
    {
        auto pointPosition = point->getPosition();
        jsonSaveVec(jsonPoints->addChild(nullptr), pointPosition);
    }

    auto jsonSegments = json->addChild("segments");
    jsonSegments->setArray();
    for (const auto& segment : m_segments)
    {
        auto jsonSegment = jsonSegments->addChild(nullptr);

        auto startPointIt = std::find(m_points.begin(), m_points.end(), segment->getStartPoint());
        jsonSegment->addChild("start_index", int(std::distance(m_points.begin(), startPointIt)));

        jsonSaveVec(jsonSegment->addChild("start_tangent"), segment->getStartTangent());
        jsonSaveVec(jsonSegment->addChild("start_up"), segment->getStartUp());

        auto endPointIt = std::find(m_points.begin(), m_points.end(), segment->getEndPoint());
        jsonSegment->addChild("end_index", int(std::distance(m_points.begin(), endPointIt)));

        jsonSaveVec(jsonSegment->addChild("end_tangent"), segment->getEndTangent());
        jsonSaveVec(jsonSegment->addChild("end_up"), segment->getEndUp());
        
    }

    return json->save(name);
}

void WorldSplineGraph::setSplineGraphName(const char* name, bool force_load)
{
    m_splineGraphName = name;
}

}//namespace MetaUnigine
