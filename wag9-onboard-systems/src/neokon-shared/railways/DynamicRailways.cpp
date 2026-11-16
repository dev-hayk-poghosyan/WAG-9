#include "DynamicRailways.h"

#include "DynamicRailwayPartGenerated.h"
#include "RailwayMeshGeneratorConstants.h"

#include "neokon-shared/CopySurfaceParametersFromTo.h"
#include "neokon-shared/metaunigine/MetaUnigine.h"
#include "neokon-shared/metaunigine/MetaUnigineObjects.h"
#include "neokon-shared/GlmUnigineInterop.h"

#include "neokon-shared/Profiler.h"

#include <UnigineConsole.h>

namespace
{
const float DynamicRailwaysExtents = 1000.0f;// This is used only to bypass Unigine's visibility check
bool dynamicRailwaysRegistred = false;
}//Anonymous namespace

DynamicRailways::DynamicRailways(void* object)
    : ObjectExternBase(object)
{
    m_asyncScheduleFunc = [](const AsyncScheduledFuncType& sheduled) -> std::future<bool>
    {
        return std::async(std::launch::async, sheduled);
    };

    setExtents(DynamicRailwaysExtents);
}

DynamicRailways::DynamicRailways(
    const DynamicRailwaysGetDataFunc& getDataFunc, std::string const& rootNodeName)
    : m_getDataFunc(getDataFunc)
    , m_rootNodeName(rootNodeName)
{
    m_asyncScheduleFunc = [](const AsyncScheduledFuncType& sheduled) -> std::future<bool>
    {
        return std::async(std::launch::async, sheduled);
    };

    setExtents(DynamicRailwaysExtents);
}

DynamicRailways::~DynamicRailways()
{
    clear();
}

void DynamicRailways::clear()
{
    // Wait for the 'updateDataInOtherThreads', started on a separate thread by the last
    // 'preRender()' call, to be finished
    if (m_lastUpdate.valid())
    {
        m_lastUpdate.wait();
    }

    // Wait for all of the loadings of future parts, started on a separate threads by the last
    // 'updateDataInOtherThreads' call, to be finished
    if (m_loadingData)
    {
        for (auto& loadingData : *m_loadingData)
        {
            loadingData.waitable.wait();
        }
        m_loadingData.reset();
    }

    m_partsData.reset();
    m_oldPartsData.reset();
    m_loadedData.clear();

    m_clustersData.reset();
    m_oldClustersData.reset();
    m_clustersToCreate.clear();
    m_createdClusters.clear();
}

void DynamicRailways::setAsyncScheduleFunc(const AsyncScheduleFuncType& func)
{
    m_asyncScheduleFunc = func;
}

void DynamicRailways::setFutureFuncToGetData(FutureDynamicRailwaysGetDataFunc&& futureGetDataFunc)
{
    m_futureGetDataFunc = std::move(futureGetDataFunc);
}

void DynamicRailways::setFuncToGetData(const DynamicRailwaysGetDataFunc& getDataFunc)
{
    m_getDataFunc = getDataFunc;
}

void DynamicRailways::setEnabled(bool enabled)
{
    m_enabled.store(enabled, std::memory_order_relaxed);
    if (!enabled)
    {
        m_readiness = std::nullopt;
    }
}

void DynamicRailways::setPartsToCreatePerFrameLimit(int count)
{
    m_partsToCreatePerFrame = count;
}

std::optional<float>
DynamicRailways::getReadiness() const
{
    return m_readiness.load(std::memory_order_relaxed);
}

void DynamicRailways::preRender(float ifps)
{
    using namespace Unigine;

    NEO_PROFILER_FUNCTION;

    auto center = getNode()->getWorldPosition();

    if (m_lastUpdate.valid())
    {
        if (m_lastUpdate.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready)
        {
            return;
        }
        if (m_lastUpdate.get())
        {
            applyDataInMainThread(center);
        }
    }

    m_lastUpdate = m_asyncScheduleFunc(
        [this, center]
        {
            return updateDataInOtherThreads(center);
        }
    );
}

void DynamicRailways::setExtents(float extents)
{
    m_boundBox.set(Unigine::Math::vec3(-extents), Unigine::Math::vec3(extents));
    m_boundSphere.set(m_boundBox);
}

bool DynamicRailways::updateDataInOtherThreads(const Unigine::Math::Vec3& center)
{
    using namespace Unigine;

    NEO_PROFILER_FUNCTION;

    if (!m_getDataFunc)
    {
        if (m_futureGetDataFunc.valid())
        {
            if (m_futureGetDataFunc.wait_for(std::chrono::milliseconds(0)) ==
                std::future_status::ready)
            {
                m_getDataFunc = m_futureGetDataFunc.get();
                m_futureGetDataFunc = {};
            }
        }
        return false;
    }

    // Swap data buffers
    m_oldPartsData = m_partsData;
    m_partsData = std::make_shared<PartsStorage>();

    auto oldLoadingData = m_loadingData;
    m_loadingData = std::make_shared<std::list<FutureLoaded>>();

    m_oldClustersData = m_clustersData;
    m_clustersData = std::make_shared<ClustersStorage>();

    if (!m_enabled.load(std::memory_order_relaxed))
    {
        return true;
    }

    std::list<DynamicRailwayPartPtr> partsToLoad;

    auto parts = queryParts(center);
    for (auto& part : parts)
    {
        if (validatePart(center, part))
        {
            continue;
        }
        
        if (validateLoading(part, oldLoadingData))
        {
            continue;
        }

        partsToLoad.push_back(part);
    }

    if (!parts.empty())
    {
        m_readiness = 1.0f - float(partsToLoad.size()) / float(parts.size());
    }
    else
    {
        m_readiness = 1.0f;
    }
    oldLoadingData.reset(); // Discard unused still loading data

    int partsToLoadLimit(std::thread::hardware_concurrency());
    partsToLoadLimit -= m_loadingData->size() + 1;
    for (auto& part : partsToLoad)
    {
        if (--partsToLoadLimit < 0)
        {
            break;
        }
        auto futurePart = m_asyncScheduleFunc(
            [this, part]
            {
                PROFILER_FMT("DynamicRailways::updateDataInOtherThreads");
                try
                {
                    if (!part->load())
                    {
                        part->unload();
                    }
                }
                catch (std::exception const& e)
                {
                    Unigine::Log::error("Exception while load() %d part: %s\n", part->getKey().id,
                        e.what());
                    // TODO: Dump part attribs for debugging (ifdef _DEBUG)
                    part->unload();
                }
                return true; // To prevent infinite loop, trying to load - always return true
            });
        auto partKey = part->getKey();
        //Debug message:
        //Unigine::Log::message("Ask to load %d - %d\n", partKey.id, partKey.subId);
        m_loadingData->push_back({partKey.key, std::move(futurePart), part});
    }

    if (m_clustersData)
    {
        for (auto& clusterData : *m_clustersData)
        {
            clusterData.second->clusters.second->updateSpatialTree();
        }
    }

    return true;
}

void DynamicRailways::ClusterData::swap(const Unigine::Math::Vec3& center)
{
    std::swap(clusters.first, clusters.second);

    clusters.first->setEnabled(true);
    clusters.first->updateIsCameraVisible();

    clusters.second->setEnabled(false);
    clusters.second->setPosition(center);// Move to 'center' to prevent float jittering on high distance
    clusters.second->clearMeshes();
}

DynamicRailways::ClusterData::~ClusterData()
{
    if (clusters.first)
    {
        clusters.first->deleteForce();
    }
    if (clusters.second)
    {
        clusters.second->deleteForce();
    }
}

void DynamicRailways::applyDataInMainThread(const Unigine::Math::Vec3& center)
{
    using namespace Unigine;

    NEO_PROFILER_FUNCTION;

    applyLoaded();

    if (m_clustersData)
    {
        for (auto& clusterData : *m_clustersData)
        {
            clusterData.second->swap(center);
        }
    }

    createRequestedClusters();

    if (m_oldPartsData)
    {
        //Debug message:
        /*for (auto partIt = m_oldPartsData->begin(); partIt != m_oldPartsData->end();
            ++partIt)
        {
            auto key = partIt->second->generated->getKey();
            Unigine::Log::message("Deleted %d - %d\n", key.id, key.subId);
        }*/
        m_oldPartsData.reset();
    }
    if (m_oldClustersData)
    {
        //Debug message:
        /*for (auto clusterIt = m_oldClustersData->begin(); clusterIt != m_oldClustersData->end();
            ++clusterIt)
        {
            Unigine::Log::message("Deleted cluster %s\n", clusterIt->first.data());
        }*/
        m_oldClustersData.reset();
    }
}

DynamicRailwayParts DynamicRailways::queryParts(const Unigine::Math::Vec3& center)
{
    auto parts = m_getDataFunc(center);

    // Sort to create nearest parts first
    std::sort(parts.begin(), parts.end(), [&center](
        const DynamicRailwayPartPtr& a, const DynamicRailwayPartPtr& b)
    {
        double const aDst = a->getBoundBox().distanceValid(center);
        double const bDst = b->getBoundBox().distanceValid(center);
        return aDst < bDst;
    });

    return parts;
}

bool DynamicRailways::validatePart(const Unigine::Math::Vec3& center,
    const DynamicRailwayPartPtr& part)
{
    auto partKey = part->getKey();
    if (m_oldPartsData)
    {
        auto partIt = m_oldPartsData->find(partKey.key);
        if (partIt != m_oldPartsData->end())
        {
            if (part->isDifferentWith(partIt->second->generated))
            {
                // Part will be deleted as non-used at this update and created again at next.
                return true;
            }

            // Keep still required data
            partIt->second->generated->validateClusters([this](const std::string& nodePath,
                const RailwayMeshGeneratorConfig::MaterialInfo& matInfo)
                {
                    return getOrCreateClusterWhilePartGeneration(nodePath, matInfo);
                }, center
            );
            m_partsData->emplace(partIt->first, partIt->second);
            m_oldPartsData->erase(partIt);
            return true;
        }
    }

    if (m_partsData)
    {
        auto partIt = m_partsData->find(partKey.key);
        if (partIt != m_partsData->end())
        {
            Unigine::Log::warning("Already moved duplicate part in 'parts' %d - %d\n",
                partKey.id, partKey.subId);
            return true;
        }
    }
    return false;
}

bool DynamicRailways::validateLoading(const DynamicRailwayPartPtr& part,
    std::shared_ptr<std::list<DynamicRailways::FutureLoaded>>& oldLoadingData)
{
    auto partKey = part->getKey();

    if (oldLoadingData)
    {
        auto futureIt = std::find_if(oldLoadingData->begin(), oldLoadingData->end(),
            [&partKey](const FutureLoaded& part)
            {
                return part.key == partKey.key;
            }
        );
        if (futureIt != oldLoadingData->end())
        {
            if (futureIt->waitable.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready)
            {
                m_loadingData->push_back(std::move(*futureIt)); // Keep still loading data
            }
            else
            {
                // Move to loaded data
                if (futureIt->waitable.get())
                {
                    //Debug message:
                    //Unigine::Log::message("Loaded %d - %d\n", partKey.id, partKey.subId);
                    m_loadedData.push_back(futureIt->part);
                }
            }
            return true;
        }
    }

    if (m_loadingData)
    {
        auto futureIt = std::find_if(m_loadingData->begin(), m_loadingData->end(),
            [&partKey](const FutureLoaded& part)
            {
                return part.key == partKey.key;
            }
        );
        if (futureIt != m_loadingData->end())
        {
            //Debug message:
            //Unigine::Log::message("Already asked to load %d - %d\n", partKey.id, partKey.subId);
            return true;
        }
    }

    auto loadedIt = std::find_if(m_loadedData.begin(), m_loadedData.end(),
        [&partKey](const DynamicRailwayPartPtr& part)
        {
            return part->getKey().key == partKey.key;
        }
    );
    if (loadedIt != m_loadedData.end())
    {
        //Debug message:
        //Unigine::Log::message("Already loaded %d - %d\n", partKey.id, partKey.subId);
        return true;
    }

    return false;
}

ObjectMultiMeshClusterPtr DynamicRailways::getOrCreateClusterWhilePartGeneration(
    const std::string& nodePath, const RailwayMeshGeneratorConfig::MaterialInfo& materialInfo)
{
    if (nodePath.empty())
    {
        Unigine::Log::error("getOrCreateCluster() with empty nodePath\n");
        return nullptr;
    }

    auto clusterName =
        nodePath + materialInfo.name + materialInfo.generateMatNameFromParams().get();

    if (m_clustersData)
    {
        auto clusterIt = m_clustersData->find(clusterName);
        if (clusterIt != m_clustersData->end())
        {
            auto result = clusterIt->second->clusters.second;
            return result;
        }
    }

    if (m_oldClustersData)
    {
        auto clusterIt = m_oldClustersData->find(clusterName);
        if (clusterIt != m_oldClustersData->end())
        {
            auto result = clusterIt->second->clusters.second;
            m_clustersData->emplace(clusterName, clusterIt->second);
            m_oldClustersData->erase(clusterIt);
            return result;
        }
    }

    auto clusterIt = std::find_if(m_createdClusters.begin(), m_createdClusters.end(),
        [&clusterName](const CreatedCluster& cluster)
        {
            return cluster.first == clusterName;
        }
    );
    if (clusterIt != m_createdClusters.end())
    {
        //Debug message:
        //Unigine::Log::message("Use created cluster %s\n", clusterIt->first.data());

        auto result = clusterIt->second->clusters.second;
        m_clustersData->emplace(clusterName, clusterIt->second);
        m_createdClusters.erase(clusterIt);
        return result;
    }

    auto createdClusterIt = std::find_if(m_clustersToCreate.begin(), m_clustersToCreate.end(),
        [&clusterName](const ClusterToCreate& cluster)
        {
            if (cluster.name == clusterName)
            {
                //Debug message:
                //Unigine::Log::message("Already asked to create cluster %s\n", clusterName.data());
                return true;
            }
            return false;
        }
    );
    if (createdClusterIt == m_clustersToCreate.end())
    {
        //Debug message:
        //Unigine::Log::message("Ask to create cluster %s\n", clusterName.data());
        m_clustersToCreate.emplace_back(ClusterToCreate{
            clusterName,
            nodePath, materialInfo
        });
    }
    return nullptr;
};

void DynamicRailways::applyLoaded()
{
    int partsToCreateLeft = m_partsToCreatePerFrame; // Limit count of objects to create in one frame
    for (auto loadedIt = m_loadedData.begin(); loadedIt != m_loadedData.end();)
    {
        auto& key = loadedIt->get()->getKey();

        auto newPart = std::make_shared<PartData>();
        newPart->generated = *loadedIt;
        if (newPart->object = loadedIt->get()->createNodes())
        {
            if (!newPart->object->getParent())
            {
                if (!m_rootNode)
                {
                    m_rootNode = Unigine::NodeDummy::create();
                    m_rootNode->setName(m_rootNodeName.c_str());
                }
                newPart->object->setParent(m_rootNode);
            }

            //Debug code for saving state for inspection:
            //newPart->object->setShowInEditorEnabled(true);
            //newPart->object->setSaveToWorldEnabled(true);
            //auto meshName = String::format("DynamicMesh_%d_%d.mesh",
            //    key.id, key.subId);
            //newPart->object->setMeshName(meshName);
            //newPart->object->saveMesh(meshName);

            //Debug message:
            //Unigine::Log::message("Created %d - %d\n", key.id, key.subId);
        }
        m_partsData->emplace(key.key, newPart);

        loadedIt = m_loadedData.erase(loadedIt);
        if ((loadedIt == m_loadedData.end()) || (!--partsToCreateLeft))
        {
            break;
        }
    }
    //Debug message:
    //Unigine::Log::message("Parts to create left %d, total %d\n", m_loadedData.size(),
    //  m_partsData->size());
}

ObjectMultiMeshClusterPtr
DynamicRailways::createCluster(const ClusterToCreate& info) const
{
    return ObjectMultiMeshCluster::create(info.nodePath.data(), info.materialInfo);
}

void DynamicRailways::createRequestedClusters()
{
    int partsToCreateLeft = m_partsToCreatePerFrame; // Limit count of clusters to create in one frame
    m_createdClusters.clear(); // Delete all unused clusters
    for (auto toCreateIt = m_clustersToCreate.begin(); toCreateIt != m_clustersToCreate.end();)
    {
        auto data = std::make_shared<ClusterData>();
        // We create pair of Unigine::ObjectMeshCluster with equal parameters to be used:
        // 'first' for rendering in main thread,
        // 'second' to be filled with transforms in separate thread
        // and to be swapped on every update

        data->clusters = std::make_pair(createCluster(*toCreateIt), createCluster(*toCreateIt));
        if (data->clusters.first && data->clusters.second)
        {
            // Debug code for saving state for inspection:
            // data->clusters.first->setSaveToWorldEnabled(true);
            // data->clusters.first->setShowInEditorEnabled(true);
            // data->clusters.second->setSaveToWorldEnabled(true);
            // data->clusters.second->setShowInEditorEnabled(true);

            // Debug message:
            // Unigine::Log::message("Created cluster %s\n", toCreateIt->first.data());
            data->clusters.second->setEnabled(false); // Disable back-buffer
            m_createdClusters.push_back(std::make_pair(toCreateIt->name, data));
        }
        else
        {
            Unigine::Log::error("Failed to create cluster for \"%s\"\n", toCreateIt->name.data());
        }

        toCreateIt = m_clustersToCreate.erase(toCreateIt);
        if ((toCreateIt == m_clustersToCreate.end()) || (!--partsToCreateLeft))
        {
            break;
        }
    }
}

void
partEnable(int argc, char** argv)
{
    int partType = (argc > 1) ? std::atoi(argv[1]) : 0;
    bool enable = (argc > 2) ? (_stricmp(argv[2], "true") == 0) : true;

    if (partType >= +DynamicRailwayPartType::Total)
    {
        Unigine::Log::message("Part type out of bounds\n");
    }
    setDynamicRailwayPartTypeEnabled(DynamicRailwayPartType(partType), enable);
}

void DynamicRailways::registerClass()
{
    if (!dynamicRailwaysRegistred)
    {
        Unigine::Console::addCommand("dynamic_railways_part_enable",
            "Enable dynamic railways parts", Unigine::MakeCallback(&partEnable));

        ObjectExternBase::addClassID<DynamicRailways>(+ObjectExternIds::DynamicRailways);

        dynamicRailwaysRegistred = true;
    }
}

DynamicRailways*
DynamicRailways::create(
    const DynamicRailwaysGetDataFunc& getDataFunc, std::string const& rootNodeName)
{
    return new DynamicRailways(getDataFunc, rootNodeName);
}
