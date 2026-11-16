#pragma once

#include "MetaUnigineNodes.h"
#include <UnigineObjects.h>

namespace MetaUnigine
{

class Object;
using ObjectPtr = std::shared_ptr<Object>;

class ObjectMeshStatic;
using ObjectMeshStaticPtr = std::shared_ptr<ObjectMeshStatic>;

class ObjectMeshSkinned;
using ObjectMeshSkinnedPtr = std::shared_ptr<ObjectMeshSkinned>;

class ObjectMeshCluster;
using ObjectMeshClusterPtr = std::shared_ptr<ObjectMeshCluster>;

/**
 * @see Unigine::Object documentation.
 */
class Object : public Node
{
public:
    Object();
    virtual Unigine::XmlPtr saveXml(const Unigine::XmlPtr& xml) const;
    virtual bool loadXml(const Unigine::XmlPtr& xml);

    int getNumSurfaces() const;
    const char* getSurfaceName(int surface) const;
    int findSurface(const char* name) const;
    const char* getMaterialName(int surface) const;
    Unigine::MaterialPtr getMaterial(int surface) const;
    int setMaterial(const Unigine::MaterialPtr& mat, int surface);
    int setMaterial(const char* name, int surface);

    void setEnabled(bool enabled, int surface);
    bool isEnabled(int surface) const;
    void setMinParent(int parent, int surface);
    int getMinParent(int surface) const;
    void setMaxParent(int parent, int surface);
    int getMaxParent(int surface) const;
    void setCastShadow(bool enabled, int surface);
    bool getCastShadow(int surface) const;
    void setCastWorldShadow(bool enabled, int surface);
    bool getCastWorldShadow(int surface) const;
    void setBakeToEnvProbe(bool enabled, int surface);
    bool getBakeToEnvProbe(int surface) const;
    void setBakeToGI(bool enabled, int surface);
    bool getBakeToGI(int surface) const;
    void setCastEnvProbeShadow(bool enabled, int surface);
    bool getCastEnvProbeShadow(int surface) const;
    void setShadowMode(Unigine::Object::SURFACE_SHADOW_MODE mode, int surface);
    Unigine::Object::SURFACE_SHADOW_MODE getShadowMode(int surface) const;
    void setShadowMask(int mask, int surface);
    int getShadowMask(int surface) const;
    void setViewportMask(int mask, int surface);
    int getViewportMask(int surface) const;
    void setCollision(bool enabled, int surface);
    bool getCollision(int surface) const;
    void setCollisionMask(int mask, int surface);
    int getCollisionMask(int surface) const;
    void setPhysicsIntersection(bool enabled, int surface);
    bool getPhysicsIntersection(int surface) const;
    void setPhysicsIntersectionMask(int mask, int surface);
    int getPhysicsIntersectionMask(int surface) const;
    void setPhysicsFriction(float value, int surface);
    float getPhysicsFriction(int surface) const;
    void setPhysicsRestitution(float value, int surface);
    float getPhysicsRestitution(int surface) const;
    void setMinVisibleDistance(float distance, int surface);
    float getMinVisibleDistance(int surface) const;
    void setMinFadeDistance(float distance, int surface);
    float getMinFadeDistance(int surface) const;
    void setMaxVisibleDistance(float distance, int surface);
    float getMaxVisibleDistance(int surface) const;
    void setMaxFadeDistance(float distance, int surface);
    float getMaxFadeDistance(int surface) const;
    void setIntersection(bool enabled, int surface);
    bool getIntersection(int surface) const;
    void setIntersectionMask(int mask, int surface);
    int getIntersectionMask(int surface) const;

    void setShowInEditorEnabledRecursive(bool enable) {}
    void setSaveToWorldEnabledRecursive(bool enable) {}

public: // MetaUnigine extensions
    Unigine::UGUID getMaterialGUID(int surface) const;
    void setMaterialGUID(const Unigine::UGUID& guid, int surface);

protected:
    void applyUnigineObjectParams(const Unigine::ObjectPtr& object) const;

protected:
    struct Surface
    {
        std::string name;
        std::string material;
        bool enabled = true;
        int min_parent = 1;
        int max_parent = 1;
        bool cast_shadow = true;
        bool cast_world_shadow = true;
        bool bake_to_env_probe = true;
        bool bake_to_gi = true;
        bool cast_env_probe_shadow = true;
        int shadow_mode = 0;
        int shadow_mask = 0x1;
        int viewport_mask = 0x1;
        bool collision = false;
        int collision_mask = 0x1;
        bool physics_intersection = false;
        int physics_intersection_mask = 0x1;
        float min_visible_distance = -std::numeric_limits<float>::max();
        float max_visible_distance = std::numeric_limits<float>::max();
        float min_fade_distance = 0.0f;
        float max_fade_distance = 0.0f;
        bool intersection = true;
        int intersection_mask = 0x1;
        float physics_friction = 0.5f;
        float physics_restitution = 0.5f;
    };
    std::vector<Surface> m_surfaces;
};

/**
 * @see Unigine::ObjectMeshStatic documentation.
 */
class ObjectMeshStatic : public Object
{
public:
    static ObjectMeshStaticPtr create();
    static ObjectMeshStaticPtr create(const char* meshPath);
    ObjectMeshStatic();
    explicit ObjectMeshStatic(const char* meshPath);

    virtual Unigine::XmlPtr saveXml(const Unigine::XmlPtr& xml) const;
    virtual bool loadXml(const Unigine::XmlPtr& xml);
    virtual Unigine::BoundBox getBoundBox() const;
    virtual UNIGINE_BOUND_BOX getWorldBoundBox() const;

    void setMeshNameForce(const char* name);
    void setMeshName(const char* name);
    const char* getMeshName() const;

    bool saveMesh(const char* path) const;
    /**
     * Set mesh, 'unique' originally controls re-using same mesh by multiple ObjectMeshStatics, but
     * it is not implemented in MetaUnigine, added here for interface compatibility.
     */
    bool setMesh(const Unigine::MeshPtr& mesh, bool unique = true);
    bool getMesh(const Unigine::MeshPtr& mesh) const;

public: // Extensions
    Unigine::ObjectMeshStaticPtr createUnigineMeshStatic() const;
    virtual bool isValid() const override;
    virtual Unigine::NodePtr createUnigine() const override;

private:
    int loadMesh();

    std::string m_meshPath;
    Unigine::MeshPtr m_mesh;
};

/**
 * @see Unigine::ObjectMeshSkinned documentation.
 */
class ObjectMeshSkinned : public Object
{
public:
    static ObjectMeshSkinnedPtr create(const char* meshPath);
    ObjectMeshSkinned();
    explicit ObjectMeshSkinned(const char* meshPath);
    virtual Unigine::XmlPtr saveXml(const Unigine::XmlPtr& xml) const;
    virtual bool loadXml(const Unigine::XmlPtr& xml);
    virtual Unigine::BoundBox getBoundBox() const;
    virtual UNIGINE_BOUND_BOX getWorldBoundBox() const;

    const char* getMeshName() const;
    void setMeshNameForce(const char* path);

    /**
     * Assign mesh, will set all surfaces materials to 'mesh_base'
     */
    int setMesh(const Unigine::MeshPtr& mesh);
    int getMesh(Unigine::MeshPtr& mesh) const;

    int getNumBones() const;
    const char* getBoneName(int bone) const;
    int findBone(const char* name) const;

    const char* getAnimName() const;
    void setAnimName(const char* name);

    int getNumAnimations() const;
    int findAnimation(const char* path) const;
    int addAnimation(const char* path);
    int getNumAnimationFrames(int animation) const;

    int addLayer();
    void removeLayer(int layer);
    void setNumLayers(int layers);
    int getNumLayers() const;
    void setLayer(int layer, bool enabled, float weight);
    void setLayerEnabled(int layer, bool enabled);
    bool isLayerEnabled(int layer) const;
    void setLayerWeight(int layer, float weight);
    float getLayerWeight(int layer) const;

    int setAnimation(int layer, int animation);
    int getAnimation(int layer) const;

    /**
     * Returns the bind pose bone transformation matrix. Bone transformations are relative. 
     */
    Unigine::Math::mat4 getBoneBindTransform(int bone) const;

    /**
     * Returns a transformation matrix of a given bone relatively to the parent object.
     * The difference from getBoneTransform() is that this method takes into account only the
     * transformation in the animation layer (no blending is done).
     */
    Unigine::Math::mat4 getBoneLayerTransform(int layer, int bone) const;

    /**
     * Returns a transformation matrix of a given bone relatively to the parent object
     * (not considering transformations of the node itself). 
     */
    Unigine::Math::mat4 getBoneTransform(int bone) const;

    /**
     * Returns the current transformation matrix applied to the bone in the world coordinate space
     * (considering node transformations). 
     */
    Unigine::Math::Mat4 getBoneWorldTransform(int bone) const;

    void setTime(float time);
    float getTime() const;
    void setSpeed(float speed);
    float getSpeed() const;
    void play();
    void stop();
    bool isPlaying() const;
    bool isStopped() const;

public: // Extensions
    /**
     * Assign mesh only if it has same surface count - to keep all the materials, otherwise return 0
     */
    int setMeshKeepMaterials(const Unigine::MeshPtr& mesh);

    Unigine::ObjectMeshSkinnedPtr createUnigineMeshSkinned() const;
    virtual bool isValid() const override;
    virtual Unigine::NodePtr createUnigine() const override;

private:
    struct AnimInfo
    {
        std::string path;
        Unigine::MeshPtr anim;
        int animIdx{0};
    };

    struct AnimLayerInfo
    {
        int animIdx{0};
        bool enabled{false};
        float weight{0.0f};
    };

    std::string m_meshPath;
    std::string m_animName;
    Unigine::MeshPtr m_mesh;
    std::vector<AnimInfo> m_animations;
    std::vector<AnimLayerInfo> m_animLayers;

    int m_fps_visible_camera = -1;
    int m_fps_visible_shadow = 30;
    int m_fps_invisible = 0;

    float m_time = 0.0f;
    float m_speed = 1.0f;
    int m_playing = 0;
};

/**
 * @see Unigine::ObjectMeshCluster documentation.
 */
class ObjectMeshCluster : public Object
{
public:
    static ObjectMeshClusterPtr create(const char* meshPath);
    ObjectMeshCluster();
    explicit ObjectMeshCluster(const char* meshPath);
    virtual Unigine::XmlPtr saveXml(const Unigine::XmlPtr& xml) const;
    virtual bool loadXml(const Unigine::XmlPtr& xml);

    void setMeshName(const char* name);
    const char* getMeshName() const;

    void setVisibleDistance(float distance);
    float getVisibleDistance() const;

    int addMeshTransform();
    Unigine::Math::mat4 getMeshTransform(int num) const;
    void setMeshTransform(int num, const Unigine::Math::mat4& transform);
    void clearMeshes();

    void updateSpatialTree() {}

public: // Extensions
    Unigine::ObjectMeshClusterPtr createUnigineCluster() const;
    virtual bool isValid() const override;
    virtual Unigine::NodePtr createUnigine() const override;

private:
    int loadMesh();

    std::string m_meshPath;
    Unigine::MeshPtr m_mesh;
    float m_visibleDistance = 0.0f; // TODO: save/load this

    std::vector<Unigine::Math::mat4> m_transforms;
};

}//namespace MetaUnigine
