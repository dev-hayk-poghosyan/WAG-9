#include "DynamicRailwayPart.h"

DynamicRailwayPart::DynamicRailwayPart(uint32_t id, uint32_t subId)
{
    m_key.id = id;
    m_key.subId = subId;
}

void
DynamicRailwayPart::setOnGenerationFinishedCallback(
    const DynamicRailwayPartFinishedFunc& callback)
{
    m_onGenerationFinishedCallback = callback;
}

void
DynamicRailwayPart::setOnUnloadCallback(const DynamicRailwayPartFinishedFunc& callback)
{
    m_onUnloadCallback = callback;
}

void DynamicRailwayPart::setBBox(const Unigine::Math::Vec3& min, const Unigine::Math::Vec3& max)
{
    Unigine::Math::Vec3 useMax(max);
    if (useMax.x <= min.x)
    {
        useMax.x = min.x + 0.1;
    }
    if (useMax.y <= min.y)
    {
        useMax.y = min.y + 0.1;
    }
    if (useMax.z <= min.z)
    {
        useMax.z = min.z + 0.1;
    }
    m_bbox.set(min, useMax);
}

void
DynamicRailwayPart::onGenerationFinished(const Unigine::NodePtr& node)
{
    m_createdNode = node;
    if (m_onGenerationFinishedCallback)
    {
        m_onGenerationFinishedCallback(shared_from_this());
    }
}

void
DynamicRailwayPart::onUnload()
{
    m_createdNode = nullptr;
    if (m_onUnloadCallback)
    {
        m_onUnloadCallback(shared_from_this());
    }
}
