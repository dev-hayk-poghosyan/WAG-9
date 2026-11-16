#include "DynamicRailwayPathBaseAdjunct.h"

#include "DynamicRailwayBranchSegment.h"

#include "neokon-shared/GenerateMeshFromSplineList.h"
#include "neokon-shared/GlmUnigineInterop.h"
#include "neokon-shared/CalcOffsetedBezierSegment.h"
#include "neokon-shared/FloatingPointCompare.h"

using namespace Unigine;

//----------------------------------------------------------------------------------
// class DynamicRailwayPathBaseAdjunct
//----------------------------------------------------------------------------------
DynamicRailwayPathBaseAdjunct::DynamicRailwayPathBaseAdjunct(uint32_t partId,
    std::string const& configPath, BezierSpline const& spline)
    : DynamicRailwayPartGenerated(partId, 0, configPath)
    , m_srcSpline(spline)
    , m_startDistanceM(0)
    , m_endDistanceM(0)
    , m_firstSegment(false)
    , m_lastSegment(false)
{
    auto const& worldAABB = spline.getAABB();
    setBBox(fromGlm(worldAABB.minCoords()),
            fromGlm(worldAABB.maxCoords()));
}

DynamicRailwayPathBaseAdjunct::DynamicRailwayPathBaseAdjunct(uint32_t partId, uint32_t segmentIndex,
    const std::string& meshGenConfigPath, double startDistanceM, double endDistanceM,
    const BezierSpline& bezierInWorld, bool firstSegment, bool lastSegment,
    const DynamicRailwayPartWithSleepers::ConstSleepersPtr& sleepers)
    : DynamicRailwayPartGenerated(partId, segmentIndex, meshGenConfigPath)
    , m_startDistanceM(startDistanceM)
    , m_endDistanceM(endDistanceM)
    , m_firstSegment(firstSegment)
    , m_lastSegment(lastSegment)
    , m_srcSpline(bezierInWorld)
    , m_sleepers(sleepers)
{
    assert(m_startDistanceM < m_endDistanceM);

    setBBox(fromGlm(bezierInWorld.getAABB().minCoords()),
        fromGlm(bezierInWorld.getAABB().maxCoords()));
}

bool
DynamicRailwayPathBaseAdjunct::isDifferentWith(const DynamicRailwayPartPtr& old) const
{
    auto oldAdjunct = std::dynamic_pointer_cast<DynamicRailwayPathBaseAdjunct>(old);
    if (!oldAdjunct)
    {
        return true;
    }

    if (!CmDoubleCompare::equal(m_startDistanceM, oldAdjunct->m_startDistanceM))
    {
        return true;
    }

    if (!CmDoubleCompare::equal(m_endDistanceM, oldAdjunct->m_endDistanceM))
    {
        return true;
    }

    if (m_srcSpline.isDifferentWith(oldAdjunct->m_srcSpline))
    {
        return true;
    }

    return DynamicRailwayPartGenerated::isDifferentWith(old);
}

void DynamicRailwayPathBaseAdjunct::unload()
{
    m_mesh.clear();
}

void DynamicRailwayPathBaseAdjunct::addTransformsToCluster(ObjectMultiMeshClusterPtr const& cluster,
                                                       std::optional<Math::Vec3> const& genAreaCenter,
                                                       std::vector<Math::Mat4>  const& transforms) const
{

    double visibleDistance = cluster->getVisibleDistance();
    auto invClusterTransform = Math::inverse(cluster->getWorldTransform());

    for (Math::Mat4 const& nodeTransform : transforms)
    {
        bool skip = genAreaCenter.has_value();
        if (genAreaCenter)
        {
            double distance = ((*genAreaCenter) - nodeTransform.getTranslate()).length();
            skip = distance > visibleDistance;
        }

        if (skip)
        {
            continue;
        }

        Math::mat4 meshTransform = Math::mat4(invClusterTransform * nodeTransform);
        cluster->setMeshTransform(cluster->addMeshTransform(), meshTransform);
    }
}

int DynamicRailwayPathBaseAdjunct::getOrCreateMeshSurface(const char* name)
{
    int surfaceIndex = m_mesh->findSurface(name);
    if (surfaceIndex >= 0)
    {
        return surfaceIndex;
    }

    return m_mesh->addSurface(name);
}

ObjectMeshStaticPtr DynamicRailwayPathBaseAdjunct::doCreateNode(const char* name) const
{
    auto result = ObjectMeshStatic::create(m_mesh);

    result->setName(name);
    result->setWorldTransform(m_nodeTransform);

    return result;
}
