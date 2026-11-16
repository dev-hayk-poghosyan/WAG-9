#pragma once

#include <UnigineMathLib.h>
#include <UnigineObjects.h>

#include <functional>
#include <memory>
#include <optional>
#include <string>

#include "neokon-shared/railways/RailwayMeshGeneratorConfig.h"
#include "neokon-shared/railways/ObjectMultiMeshCluster.h"

class DynamicRailwayPart;
using DynamicRailwayPartPtr = std::shared_ptr<DynamicRailwayPart>;
using DynamicRailwayPartLink = std::weak_ptr<DynamicRailwayPart>;

using DynamicRailwayPartFinishedFunc = std::function<void(const DynamicRailwayPartPtr& part)>;

/**
 * This class is used by 'DynamicRailways' to load/unload and create nearby objects in runtime 
 */
class DynamicRailwayPart : public std::enable_shared_from_this<DynamicRailwayPart>
{
public:
    union Key {
        struct {
            uint32_t subId;
            uint32_t id;
        };
        uint64_t key;
    };

    using GetOrCreateClusterFunc =
        std::function<ObjectMultiMeshClusterPtr(const std::string&,
            const RailwayMeshGeneratorConfig::MaterialInfo&)>;

public:
    DynamicRailwayPart(uint32_t id, uint32_t subId);
    virtual ~DynamicRailwayPart() = default;

    /**
     * Get part key
     */
    const Key& getKey() const { return m_key; }

    /**
     * Get part bounding box
     */
    const UNIGINE_BOUND_BOX& getBoundBox() const { return m_bbox; }

    /**
     * Set callback to be called in main thread after creation of Unigine objects
     */
    void setOnGenerationFinishedCallback(const DynamicRailwayPartFinishedFunc& callback);

    /**
     * Set callback to be called in main thread after unload of Unigine objects
     */
    void setOnUnloadCallback(const DynamicRailwayPartFinishedFunc& callback);

    /**
     * Load data blocking function. Called in separate thread
     */
    virtual bool load() = 0;

    /**
     * UnLoad data blocking function. Called in main thread only
     */
    virtual void unload() = 0;

    /**
     * Create Unigine nodes. Called in main thread only
     */
    virtual Unigine::NodePtr createNodes() = 0;

    /**
     * Get nodes if they was already 'createNodes()', but not yet 'unload()'
     */
    virtual Unigine::NodePtr getCreatedNodes() { return m_createdNode; }

    /**
     * Check, if part have to be reloaded because it was changed.
     * 'false' = no need to reload. 
     */
    virtual bool isDifferentWith(const DynamicRailwayPartPtr& old) const { return false; }

    /**
     * Fill Unigine clusters, obtained by template node name and optional replacement material name 
     * from 'getOrCreateCluster' call, with transformations. If 'center' arg is provided, it will be
     * used for visibility distance check to only add visible sleepers.
     */
    virtual void validateClusters(const GetOrCreateClusterFunc& getOrCreateCluster,
        const std::optional<Unigine::Math::Vec3>& center = std::nullopt) const {}

protected:
    Key m_key;
    UNIGINE_BOUND_BOX m_bbox;
    DynamicRailwayPartFinishedFunc m_onGenerationFinishedCallback;
    DynamicRailwayPartFinishedFunc m_onUnloadCallback;
    Unigine::NodePtr m_createdNode;

    void onGenerationFinished(const Unigine::NodePtr& node);
    void onUnload();

    /**
     * Set BBox, zero or negative size dimensions are not supported by boost::r_tree -
     * they will be set to '0.1' in such case 
     */
    void setBBox(const Unigine::Math::Vec3& min, const Unigine::Math::Vec3& max);
};
