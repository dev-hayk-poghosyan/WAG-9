#pragma once

#include <memory>
#include <vector>

#include <UnigineMathLib.h>
#include <UnigineObjects.h>

#include "neokon-shared/railways/RailwayMeshGeneratorConfig.h"

/**
 * This class is intended to decorate creation of multiple Unigine::ObjectMeshCluster objects from
 * node template file with multiple Unigine::ObjectMeshStatic objects in it.
 *
 * It is not inherited from Unigine Nodes/Objects, and it is not part of Unigine scene.
 * Created clusters will not be deleted in the object's destructor, 
 * to destroy them 'deleteForce' method must be called explicitly.
 */
class ObjectMultiMeshCluster
{
public:
    /**
     * Enables or disables the node.
     * Arguments:
     *  bool enabled - true to enable the node, false to disable it.
     */
    void setEnabled(bool enabled);

    /**
     * Sets the node position.
     * Arguments:
     *  const Math::Vec3& pos - Node position in the local space
     */
    void setPosition(const Unigine::Math::Vec3& pos);

    /**
     * Delete decorated clusters from Unigine scene.
     */
    void deleteForce();

    /**
     * Sets a name for the node.
     * Arguments:
     *  const char* name - New name of the node.
     */
    void setName(const char* name);

    /**
     * Sets the new parent for the node. Transformations of the current node will be done in the
     * coordinates of the parent.
     * Arguments:
     *  const Ptr<Node>& parent - New parent node.
     */
    void setParent(const Unigine::NodePtr& node);

    /**
     * Sets a value indicating if displaying in the World Hierarchy window of the UnigineEditor is
     * enabled for the node. The node shall be displayed in the hierarchy only if this option is
     * enabled for all of its ancestors as well.
     * Arguments:
     *  bool enabled - true to enable displaying in the World Hierarchy window of the UnigineEditor
     *  for the node; 0 to disable.
     */
    void setShowInEditorEnabled(bool value);

    /**
     * Sets a value indicating if saving to *.world file is enabled for the node. The node shall be
     * saved to a *.world file only if this option is enabled for all of its ancestors as well.
     * Arguments:
     *  bool enabled - true to enable saving to *.world file for the node; 0 to disable.
     */
    void setSaveToWorldEnabled(bool value);

    /**
     * Returns the transformation matrix of the node in the world coordinates.
     * Return value:
     *  Transformation matrix.
     */
    Unigine::Math::Mat4 getWorldTransform() const;

    /**
     * Returns the distance up to which meshes handled by the mesh cluster are rendered. The default
     * is infinity (in this case, fade distance is ignored).
     * Return value:
     *  Visibility distance for meshes in units.
     */
    float getVisibleDistance() const;

    /**
     * Adds a new mesh instance transformation to the cluster. To set the new added transformation
     * pass the return value of this method to the setMeshTransform() method.
     * Return value:
     *  Number of the last added mesh instance.
     */ 
    int addMeshTransform();

    /**
     * Sets the transformation for the given mesh instance.
     * Arguments:
     *  int num - Mesh instance number.
     *  const Math::mat4 & transform - Mesh transformation matrix.
     */
    void setMeshTransform(int num, const Unigine::Math::mat4& transform);

    /**
     * Updates the spatial tree of the mesh cluster. Spatial tree is updated in the next frame,
     * this method can be used to update the tree immediately.
     */
    void updateSpatialTree();

    /**
     * Refresh check for instances visibility.
     */
    void updateIsCameraVisible();

    /**
     * Deletes all meshes currently baked into mesh cluster.
     */
    void clearMeshes();

    /**
     * ObjectMultiMeshCluster constructor.
     * Arguments:
     *  const char* nodeFilePath - Name of the template node file.
     *  bool materialInfo - Replacement material parameters to be used while creation of temporary
     *  material.
     */
    static std::shared_ptr<ObjectMultiMeshCluster> create(const char* nodeFilePath,
        const RailwayMeshGeneratorConfig::MaterialInfo& materialInfo = {});

private:
    struct ClusterData
    {
        Unigine::ObjectMeshClusterPtr node;
        Unigine::Math::mat4 addTransform;
    };

    ObjectMultiMeshCluster(){}

    std::vector<ClusterData> m_clusters;
    float m_visibleDistance = 0.0f;
    Unigine::Math::Mat4 m_transform;

};

using ObjectMultiMeshClusterPtr = std::shared_ptr<ObjectMultiMeshCluster>;
