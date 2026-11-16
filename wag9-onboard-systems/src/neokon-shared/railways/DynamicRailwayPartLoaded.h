#pragma once

#include "DynamicRailwayPart.h"

#include "neokon-shared/BezierSpline.h"

#include <mutex>
#include <vector>

/**
 * This class is used by 'DynamicRailways' to load nearby railway nodes in runtime, it uses async
 * node loading.
 */
class DynamicRailwayPartLoaded : public DynamicRailwayPart
{
public:
    DynamicRailwayPartLoaded(uint32_t id, uint32_t subId, const std::string& nodePath,
        const Unigine::Math::Mat4& transform, const std::vector<BezierSpline>& splines);
    virtual ~DynamicRailwayPartLoaded();

    virtual bool load() override;
    virtual void unload() override;
    virtual Unigine::NodePtr createNodes() override;

protected:
    const std::string m_nodePath;
    Unigine::Math::Mat4 m_transform;
    Unigine::NodePtr m_node;
};
