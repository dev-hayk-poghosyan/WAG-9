#include "ObjectMultiMeshCluster.h"

#include <list>

#include "neokon-shared/metaunigine/MetaUnigine.h"
#include "neokon-shared/metaunigine/MetaUnigineObjects.h"

#include "neokon-shared/CopySurfaceParametersFromTo.h"

#include <UnigineString.h>

namespace
{

const float MinimumClusterVisibleDistance = 100.0f;

struct TemplateInfo
{
    MetaUnigine::ObjectMeshStaticPtr node;
    Unigine::Math::mat4 transform;
};

void
buildTemplateNodesList(const MetaUnigine::NodePtr& node, const Unigine::Math::mat4& transform,
    std::list<TemplateInfo>& nodes)
{
    auto nodeTransform = transform * Unigine::Math::mat4(node->getTransform());

    if (node->getType() == Unigine::Node::NODE_REFERENCE)
    {
        auto nodeReference = std::static_pointer_cast<MetaUnigine::NodeReference>(node);
        if (auto refNode = nodeReference->getReference())
        {
            buildTemplateNodesList(refNode, nodeTransform, nodes);
        }
    }

    if (node->getType() == Unigine::Node::OBJECT_MESH_STATIC)
    {
        auto objectMeshStatic = std::static_pointer_cast<MetaUnigine::ObjectMeshStatic>(node);
        nodes.emplace_back(TemplateInfo{objectMeshStatic, nodeTransform});
    }

    int numChildren = node->getNumChildren();
    for (int childIdx = 0; childIdx < numChildren; ++childIdx)
    {
        auto childNode = node->getChild(childIdx);
        buildTemplateNodesList(childNode, nodeTransform, nodes);
    }
}

} // anonymous namespace

void
ObjectMultiMeshCluster::setEnabled(bool enabled)
{
    for (const auto& cluster : m_clusters)
    {
        cluster.node->setEnabled(enabled);
    }
}

void
ObjectMultiMeshCluster::updateSpatialTree()
{
    for (const auto& cluster : m_clusters)
    {
        cluster.node->updateSpatialTree();
    }
}

void
ObjectMultiMeshCluster::updateIsCameraVisible()
{
    for (const auto& cluster : m_clusters)
    {
        cluster.node->setPosition(cluster.node->getPosition());
    }
}

void
ObjectMultiMeshCluster::setPosition(const Unigine::Math::Vec3& pos)
{
    for (const auto& cluster : m_clusters)
    {
        cluster.node->setPosition(pos);
    }
    m_transform.setTranslate(pos);
}

void
ObjectMultiMeshCluster::clearMeshes()
{
    for (const auto& cluster : m_clusters)
    {
        cluster.node->clearMeshes();
    }
}

void
ObjectMultiMeshCluster::deleteForce()
{
    for (auto& cluster : m_clusters)
    {
        cluster.node.deleteForce();
    }
    m_clusters.clear();
}

void
ObjectMultiMeshCluster::setName(const char* name)
{
    for (int clusterIdx = 0; clusterIdx < m_clusters.size(); ++clusterIdx)
    {
        const auto& cluster = m_clusters[clusterIdx];
        cluster.node->setName(Unigine::String::format("%s_%d", name, clusterIdx));
    }
}

void
ObjectMultiMeshCluster::setParent(const Unigine::NodePtr& node)
{
    for (const auto& cluster : m_clusters)
    {
        cluster.node->setParent(node);
    }
}

void
ObjectMultiMeshCluster::setShowInEditorEnabled(bool value)
{
    for (const auto& cluster : m_clusters)
    {
        cluster.node->setShowInEditorEnabled(value);
    }
}

void
ObjectMultiMeshCluster::setSaveToWorldEnabled(bool value)
{
    for (const auto& cluster : m_clusters)
    {
        cluster.node->setSaveToWorldEnabled(value);
    }
}

float
ObjectMultiMeshCluster::getVisibleDistance() const
{
    return m_visibleDistance;
}

Unigine::Math::Mat4
ObjectMultiMeshCluster::getWorldTransform() const
{
    return m_transform;
}

void
ObjectMultiMeshCluster::setMeshTransform(int num, const Unigine::Math::mat4& transform)
{
    for (const auto& cluster : m_clusters)
    {
        cluster.node->setMeshTransform(num, transform * cluster.addTransform);
    }
}

int
ObjectMultiMeshCluster::addMeshTransform()
{
    int res = 0;
    for (const auto& cluster : m_clusters)
    {
        if (cluster.node.isValid())
        {
            // Return number of the last added mesh instance, which is the same for all sub clusters
            // So, take any of them here as 'res'.
            res = cluster.node->addMeshTransform();
        }
    }
    return res;
}

std::shared_ptr<ObjectMultiMeshCluster>
ObjectMultiMeshCluster::create(const char* nodeFilePath,
    const RailwayMeshGeneratorConfig::MaterialInfo& materialInfo)
{
    MetaUnigine::NodePtr rootNode = MetaUnigine::loadNode(nodeFilePath);
    if (!rootNode)
    {
        return nullptr;
    }

    std::list<TemplateInfo> templateNodes;
    buildTemplateNodesList(rootNode, Unigine::Math::mat4::IDENTITY, templateNodes);

    ObjectMultiMeshClusterPtr result(new ObjectMultiMeshCluster);
    for (const auto& templateInfo : templateNodes)
    {
        auto cluster = Unigine::ObjectMeshCluster::create(templateInfo.node->getMeshName());

        float clusterVisibleDistance = MinimumClusterVisibleDistance;
        const int numSurfaces = cluster->getNumSurfaces();
        for (int surfaceIndex = 0; surfaceIndex < numSurfaces; ++surfaceIndex)
        {
            copySurfaceParametersFromTo(templateInfo.node, cluster, surfaceIndex);

            if (!materialInfo.name.empty())
            {
                if (!cluster->setMaterial(materialInfo.getOrCreateMaterial(), surfaceIndex))
                {
                    Unigine::Log::error("Failed to replace material \"%s\" on cluster \"%s\"\n",
                        materialInfo.name.data(), nodeFilePath);
                }
            }

            float surfaceVisibleDistance = templateInfo.node->getMaxVisibleDistance(surfaceIndex) +
                templateInfo.node->getMaxFadeDistance(surfaceIndex);
            clusterVisibleDistance = std::max(clusterVisibleDistance, surfaceVisibleDistance);
        }
        cluster->setVisibleDistance(clusterVisibleDistance);

        result->m_clusters.emplace_back(ClusterData{cluster, templateInfo.transform});
        result->m_visibleDistance = std::max(clusterVisibleDistance, result->m_visibleDistance);
    }
    return result;
}
