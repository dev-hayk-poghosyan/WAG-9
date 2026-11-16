#include "DynamicRailwayPartLoaded.h"

#include "neokon-shared/GlmUnigineInterop.h"

#include <UnigineNodes.h>
using namespace Unigine;

DynamicRailwayPartLoaded::DynamicRailwayPartLoaded(uint32_t id, uint32_t subId,
    const std::string& nodePath, const Math::Mat4& transform, const std::vector<BezierSpline>& splines)
    : DynamicRailwayPart(id, subId)
    , m_nodePath(nodePath)
    , m_transform(transform)
{
    assert(!splines.empty());

    AlignedBox<glm::dvec3> aabb = splines[0].getAABB();

    size_t const totalSplines = splines.size();
    for (size_t splineIdx = 1; splineIdx < totalSplines; ++splineIdx)
    {
        aabb |= splines[splineIdx].getAABB();
    }

    setBBox(fromGlm(aabb.minCoords()), fromGlm(aabb.maxCoords()));
    m_bbox.setTransform(m_transform);
}

DynamicRailwayPartLoaded::~DynamicRailwayPartLoaded()
{
}

bool DynamicRailwayPartLoaded::load()
{
    return true;
}

void DynamicRailwayPartLoaded::unload()
{
}

NodePtr DynamicRailwayPartLoaded::createNodes()
{
    m_node = NodeReference::create(m_nodePath.data());
    if (!m_node)
    {
        return {};
    }

    m_node->setTransform(m_transform);
    m_node->setName(String::format("junction_%d", m_key.id));
    return m_node;
}
