#pragma once

#include "DynamicRailwayPart.h"

#include "neokon-shared/PlusEnum.h"
#include "neokon-shared/ObjectExternIds.h"
#include "neokon-shared/railways/RailwayMeshGeneratorConfig.h"
#include "neokon-shared/railways/ObjectMultiMeshCluster.h"

#include <UnigineMathLib.h>
#include <UnigineBounds.h>
#include <UnigineObjects.h>

#include <memory>
#include <future>
#include <functional>

using DynamicRailwayParts = std::vector<DynamicRailwayPartPtr>;
using DynamicRailwaysGetDataFunc = std::function<DynamicRailwayParts(const Unigine::Math::Vec3& center)>;
using FutureDynamicRailwaysGetDataFunc = std::future<DynamicRailwaysGetDataFunc>;

/**
 * This class is intended to dinamically load/unload objects near it's location
 * Objects are queried by the 'getDataFunc' callback
 * 
 * It is inherited from 'Unigine::ObjectExternBase' for better Unigine integration:
 * 1) It can be created and work in Unigne Editor
 * 2) Updating it every frame is done by Unigine via 'preRender' callback
 * 3) Moving this object with camera can be done by adding it to 'Unigine::Player' as a child
 */
class DynamicRailways : public Unigine::ObjectExternBase
{
public:
    using AsyncScheduledFuncType = std::function<bool()>;
    using AsyncScheduleFuncType = std::function<std::future<bool>(const AsyncScheduledFuncType&)>;

public:
    static void registerClass();
    static DynamicRailways* create(const DynamicRailwaysGetDataFunc& getDataFunc,
        std::string const& rootNodeName = "DynamicRailwayParts");

    // This constructor is used by Unigine internal deserialization.
    DynamicRailways(void* object);
    // This is used by create(...).
    DynamicRailways(const DynamicRailwaysGetDataFunc& getDataFunc,
        std::string const& rootNodeName);
    virtual ~DynamicRailways();

    virtual void preRender(float ifps) override;

    virtual int getClassID() override { return +ObjectExternIds::DynamicRailways; }
    virtual int getNumSurfaces() override { return 0; }
    virtual const char* getSurfaceName(int surface) override { return ""; }
    virtual const Unigine::BoundBox& getBoundBox(int surface) override { return m_boundBox; }
    virtual const Unigine::BoundSphere& getBoundSphere(int surface) override { return m_boundSphere; }
    virtual const Unigine::BoundBox& getBoundBox() override { return m_boundBox; }
    virtual const Unigine::BoundSphere& getBoundSphere() override { return m_boundSphere; }

    /**
     * Destroy all parts and restart generation. 
     */
    void clear();

    /**
     * Replace default std::async(std::launch::async with own implementation to use, as example,
     * own thread pool.
     */
    void setAsyncScheduleFunc(const AsyncScheduleFuncType& func);

    /**
     * Indirectly set the getDataFunc, which is not ready yet - will be ready in future.
     * If 'm_getDataFunc' is empty - the 'm_futureGetDataFunc' is checked to be valid and ready
     * (to be assigned to 'm_getDataFunc' and cleared) while update.
     */
    void setFutureFuncToGetData(FutureDynamicRailwaysGetDataFunc&& futureGetDataFunc);

    /**
     * Set the 'm_getDataFunc' - function, called in separate thread while update to query parts to
     * be shown.
     */
    void setFuncToGetData(const DynamicRailwaysGetDataFunc& getDataFunc);

    bool getEnabled() const { return m_enabled; }
    void setEnabled(bool enabled);

    /**
     * Limit the count of parts, added to the scene per single update call (render frame),
     * default is "no limit" <= 0.
     */
    void setPartsToCreatePerFrameLimit(int count);

    /**
     * Get current percentage of readiness loading parts. If railways enabled,
     * @returns 0.0f - 1.0f the fraction of currently loaded parts, i.e. one minus fraction of parts
     * to load div total parts. Otherwise, @returns std::nullopt. This can take place before first
     * query parts call finished or after setEnabled(false).
     */
    std::optional<float> getReadiness() const;

private:
    Unigine::BoundBox m_boundBox;
    Unigine::BoundSphere m_boundSphere;
    Unigine::NodePtr m_rootNode;

    std::atomic<bool> m_enabled{ true };
    int m_partsToCreatePerFrame{-1};
    FutureDynamicRailwaysGetDataFunc m_futureGetDataFunc;
    DynamicRailwaysGetDataFunc m_getDataFunc;
    std::atomic<std::optional<float>> m_readiness;
    std::future<bool> m_lastUpdate;

    /**
     * Structure for part data storage and destruction management
     * 'generated' - data generator
     * 'object' - Unigine's object
     */
    class PartData
    {
    public:
        DynamicRailwayPartPtr generated;
        Unigine::NodePtr object;
        ~PartData() { if(generated) generated->unload(); object.deleteForce(); }
    };

    using PartsStorage = std::unordered_map<uint64_t, std::shared_ptr<PartData>>;
    std::shared_ptr<PartsStorage> m_partsData; // loaded and created
    std::shared_ptr<PartsStorage> m_oldPartsData; // to be deleted in main thread

    struct FutureLoaded
    {
        uint64_t key{0};
        std::future<bool> waitable;
        DynamicRailwayPartPtr part;
    };

    std::shared_ptr<std::list<FutureLoaded>> m_loadingData; // loading in progress

    std::list<DynamicRailwayPartPtr> m_loadedData; // loaded, not created

    /**
     * Structure for cluster data storage and destruction management
     * 'clusters' - Unigine's clusters, first to render, second to update on separate thread,
     * the first and second items are swapped during each update call.
     */
    class ClusterData
    {
    public:
        std::pair<ObjectMultiMeshClusterPtr, ObjectMultiMeshClusterPtr> clusters;
        void swap(const Unigine::Math::Vec3& center);
        ~ClusterData();
    };
    using ClustersStorage = std::unordered_map<std::string, std::shared_ptr<ClusterData>>;
    std::shared_ptr<ClustersStorage> m_clustersData; // loaded and created
    std::shared_ptr<ClustersStorage> m_oldClustersData; // to be deleted in main thread

    struct ClusterToCreate
    {
        std::string name;
        std::string nodePath;
        RailwayMeshGeneratorConfig::MaterialInfo materialInfo;
    };
    std::list<ClusterToCreate> m_clustersToCreate; // loaded, wait to be created in main thread
    using CreatedCluster = std::pair<std::string, std::shared_ptr<ClusterData>>;
    std::list<CreatedCluster> m_createdClusters; // created in main thread, wait to be filled

    AsyncScheduleFuncType m_asyncScheduleFunc;

private:
    void setExtents(float extents);

    bool updateDataInOtherThreads(const Unigine::Math::Vec3& center);
    void applyDataInMainThread(const Unigine::Math::Vec3& center);

    // Other threads
    DynamicRailwayParts queryParts(const Unigine::Math::Vec3& center);
    bool validatePart(const Unigine::Math::Vec3& center, const DynamicRailwayPartPtr& part);
    bool validateLoading(const DynamicRailwayPartPtr& part,
        std::shared_ptr<std::list<FutureLoaded>>& oldLoadingData);

    ObjectMultiMeshClusterPtr getOrCreateClusterWhilePartGeneration(const std::string& nodePath,
        const RailwayMeshGeneratorConfig::MaterialInfo& materialInfo);
    ObjectMultiMeshClusterPtr createCluster(const ClusterToCreate& info) const;

    // Main thread
    void applyLoaded();
    void createRequestedClusters();

    std::string m_rootNodeName;
};
