#pragma once

#include "MetaUnigineNodes.h"

namespace MetaUnigine
{

class SplinePoint;
using SplinePointPtr = std::shared_ptr<SplinePoint>;

class SplineSegment;
using SplineSegmentPtr = std::shared_ptr<SplineSegment>;

class WorldSplineGraph;
using WorldSplineGraphPtr = std::shared_ptr<WorldSplineGraph>;

/**
 * @see Unigine::SplinePoint documentation.
 */
class SplinePoint
{
public:
    SplinePoint(const Unigine::Math::Vec3& position);
    void setPosition(const Unigine::Math::Vec3& position);
    Unigine::Math::Vec3 getPosition() const;

private:
    Unigine::Math::Vec3 m_position;
};

/**
 * @see Unigine::SplineSegment documentation.
 */
class SplineSegment
{
public:
    SplineSegment(const SplinePointPtr& start_point,
        const Unigine::Math::vec3& start_tangent, const Unigine::Math::vec3& start_up,
        const SplinePointPtr& end_point, const Unigine::Math::vec3& end_tangent,
        const Unigine::Math::vec3& end_up);

    void setStartPoint(const SplinePointPtr& point);
    SplinePointPtr getStartPoint() const;
    void setEndPoint(const SplinePointPtr& point);
    SplinePointPtr getEndPoint() const;
    void setStartTangent(const Unigine::Math::vec3& tangent);
    Unigine::Math::vec3 getStartTangent() const;
    void setStartUp(const Unigine::Math::vec3& up);
    Unigine::Math::vec3 getStartUp() const;
    void setEndTangent(const Unigine::Math::vec3& tangent);
    Unigine::Math::vec3 getEndTangent() const;
    void setEndUp(const Unigine::Math::vec3& up);
    Unigine::Math::vec3 getEndUp() const;

private:
    SplinePointPtr m_start_point;
    Unigine::Math::vec3 m_start_tangent;
    Unigine::Math::vec3 m_start_up;
    SplinePointPtr m_end_point;
    Unigine::Math::vec3 m_end_tangent;
    Unigine::Math::vec3 m_end_up;
};

/**
 * @see Unigine::WorldSplineGraph documentation.
 */
class WorldSplineGraph : public Node
{
public:
    static WorldSplineGraphPtr create();
    WorldSplineGraph();
    virtual Unigine::XmlPtr saveXml(const Unigine::XmlPtr& xml) const;
    virtual bool loadXml(const Unigine::XmlPtr& xml);

    SplinePointPtr createSplinePoint(const Unigine::Math::Vec3& position);
    SplineSegmentPtr createSplineSegment(const SplinePointPtr& start_point, 
        const Unigine::Math::vec3& start_tangent, const Unigine::Math::vec3& start_up, 
        const SplinePointPtr& end_point, const Unigine::Math::vec3& end_tangent, 
        const Unigine::Math::vec3& end_up);

    bool save(const char* name);
    void setSplineGraphName(const char* name, bool force_load = false);

private:
    std::string m_splineGraphName;

    std::vector<SplinePointPtr> m_points;
    std::vector<SplineSegmentPtr> m_segments;
};

}//namespace MetaUnigine
