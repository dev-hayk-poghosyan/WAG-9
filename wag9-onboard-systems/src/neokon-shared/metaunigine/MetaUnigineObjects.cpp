#include "MetaUnigineObjects.h"
#include "MetaUnigineUtils.h"

#include "neokon-shared/CopySurfaceParametersFromTo.h"

#include <mutex>
#include <random>
#include <unordered_set>
#include <sstream>

#include <UnigineString.h>

namespace MetaUnigine
{

// Object

Object::Object()
{
}

#define SAVE_IF_NOT_DEFAULT(param) if (surface.param != defaults.param) xmlSurface->setArg(#param, \
        std::to_string(surface.param).data());

#define SAVE_IF_NOT_DEFAULT_FLOAT(param) if (surface.param != defaults.param) xmlSurface->setArg(#param, \
        Unigine::String::ftoa(surface.param, floatPrecision));

#define SAVE_IF_NOT_DEFAULT_HEX(param) if (surface.param != defaults.param) xmlSurface->setArg(#param, \
        (std::stringstream() << "0x" << std::hex << surface.param).str().data());

#define SAVE_IF_NOT_DEFAULT_BOOL(param) if (surface.param != defaults.param) xmlSurface->setArg(#param, \
        surface.param ? "1" : "0");

Unigine::XmlPtr Object::saveXml(const Unigine::XmlPtr& xmlNode) const
{
    int floatPrecision = 12;

    Surface defaults;
    for (const auto& surface : m_surfaces)
    {
        auto xmlSurface = xmlNode->addChild(Unigine::Xml::create("surface"));
        xmlSurface->setArg("name", surface.name.data());
        
        SAVE_IF_NOT_DEFAULT_BOOL(enabled);
        SAVE_IF_NOT_DEFAULT(min_parent);
        SAVE_IF_NOT_DEFAULT(max_parent);
        SAVE_IF_NOT_DEFAULT_BOOL(cast_shadow);
        SAVE_IF_NOT_DEFAULT_BOOL(cast_world_shadow);
        SAVE_IF_NOT_DEFAULT_BOOL(bake_to_env_probe);
        SAVE_IF_NOT_DEFAULT_BOOL(bake_to_gi);
        SAVE_IF_NOT_DEFAULT_BOOL(cast_env_probe_shadow);
        SAVE_IF_NOT_DEFAULT(shadow_mode); // ?
        SAVE_IF_NOT_DEFAULT_HEX(shadow_mask);
        SAVE_IF_NOT_DEFAULT_HEX(viewport_mask);
        SAVE_IF_NOT_DEFAULT_BOOL(collision);
        SAVE_IF_NOT_DEFAULT_HEX(collision_mask);
        SAVE_IF_NOT_DEFAULT_BOOL(physics_intersection);
        SAVE_IF_NOT_DEFAULT_HEX(physics_intersection_mask);
        SAVE_IF_NOT_DEFAULT_FLOAT(min_visible_distance);
        SAVE_IF_NOT_DEFAULT_FLOAT(max_visible_distance);
        SAVE_IF_NOT_DEFAULT_FLOAT(min_fade_distance);
        SAVE_IF_NOT_DEFAULT_FLOAT(max_fade_distance);
        SAVE_IF_NOT_DEFAULT_BOOL(intersection);
        SAVE_IF_NOT_DEFAULT_HEX(intersection_mask);
        SAVE_IF_NOT_DEFAULT_FLOAT(physics_friction);
        SAVE_IF_NOT_DEFAULT_FLOAT(physics_restitution);

        xmlSurface->setArg("material", surface.material.data());
    }

    return Node::saveXml(xmlNode);
}

#define LOAD_ARG_IF_EXISTS(param) { int paramIdx = xmlChild->findArg(#param); if (paramIdx != -1) \
                                    surface.param = xmlChild->getArgValue(paramIdx); }

#define LOAD_ARG_IF_EXISTS_INT(param) { int paramIdx = xmlChild->findArg(#param); if (paramIdx != -1) \
                                        surface.param = std::atoi(xmlChild->getArgValue(paramIdx)); }

#define LOAD_ARG_IF_EXISTS_FLOAT(param) { int paramIdx = xmlChild->findArg(#param); if (paramIdx != -1) \
                                        surface.param = std::atof(xmlChild->getArgValue(paramIdx)); }

#define LOAD_ARG_IF_EXISTS_BOOL(param) { int paramIdx = xmlChild->findArg(#param); if (paramIdx != -1) \
                                        surface.param = (std::atoi(xmlChild->getArgValue(paramIdx)) == 1); }

bool Object::loadXml(const Unigine::XmlPtr& xml)
{
    for (int childIdx = 0; childIdx < xml->getNumChildren(); ++childIdx)
    {
        auto xmlChild = xml->getChild(childIdx);
        if (!strcmp(xmlChild->getName(), "surface"))
        {
            int nameIdx = xmlChild->findArg("name");
            if (nameIdx < 0)
            {
                continue;
            }
            const char* name = xmlChild->getArgValue(nameIdx);

            auto surfaceIt = std::find_if(m_surfaces.begin(), m_surfaces.end(),
                [&name](const Surface& surface) { return surface.name == name; });
            if (surfaceIt == m_surfaces.end())
            {
                continue;
            }
            auto& surface = *surfaceIt;

            LOAD_ARG_IF_EXISTS(material);
            LOAD_ARG_IF_EXISTS_BOOL(enabled);
            LOAD_ARG_IF_EXISTS_INT(min_parent);
            LOAD_ARG_IF_EXISTS_INT(max_parent);
            LOAD_ARG_IF_EXISTS_BOOL(cast_shadow);
            LOAD_ARG_IF_EXISTS_BOOL(cast_world_shadow);
            LOAD_ARG_IF_EXISTS_BOOL(bake_to_env_probe);
            LOAD_ARG_IF_EXISTS_BOOL(bake_to_gi);
            LOAD_ARG_IF_EXISTS_BOOL(cast_env_probe_shadow);
            LOAD_ARG_IF_EXISTS_INT(shadow_mode);
            LOAD_ARG_IF_EXISTS_INT(shadow_mask);
            LOAD_ARG_IF_EXISTS_INT(viewport_mask);
            LOAD_ARG_IF_EXISTS_BOOL(collision);
            LOAD_ARG_IF_EXISTS_INT(collision_mask);
            LOAD_ARG_IF_EXISTS_BOOL(physics_intersection);
            LOAD_ARG_IF_EXISTS_INT(physics_intersection_mask);
            LOAD_ARG_IF_EXISTS_FLOAT(min_visible_distance);
            LOAD_ARG_IF_EXISTS_FLOAT(max_visible_distance);
            LOAD_ARG_IF_EXISTS_FLOAT(min_fade_distance);
            LOAD_ARG_IF_EXISTS_FLOAT(max_fade_distance);
            LOAD_ARG_IF_EXISTS_BOOL(intersection);
            LOAD_ARG_IF_EXISTS_INT(intersection_mask);
            LOAD_ARG_IF_EXISTS_FLOAT(physics_friction);
            LOAD_ARG_IF_EXISTS_FLOAT(physics_restitution);
        }
    }
    return Node::loadXml(xml);
}

int Object::getNumSurfaces() const
{
    return m_surfaces.size();
}

const char* Object::getSurfaceName(int surface) const
{
    return m_surfaces[surface].name.data();
}

int Object::findSurface(const char* name) const
{
    for (int surfaceIndex = 0; surfaceIndex < m_surfaces.size(); ++surfaceIndex)
    {
        const auto& surface = m_surfaces[surfaceIndex];
        if (surface.name == name)
        {
            return surfaceIndex;
        }
    }
    return -1;
}

const char* Object::getMaterialName(int surface) const 
{ 
    return m_surfaces[surface].material.data();
}

Unigine::MaterialPtr Object::getMaterial(int surface) const
{
    return Unigine::Materials::findMaterialByGUID(getMaterialGUID(surface));
}

int Object::setMaterial(const Unigine::MaterialPtr& mat, int surface)
{
    if (!mat)
    {
        m_surfaces[surface].material.clear();
    }
    else
    {
        m_surfaces[surface].material = mat->getGUID().getString();
    }
    return 0;
}

int Object::setMaterial(const char* name, int surface)
{
    Unigine::UGUID guid;
    guid.setFileSystemString(name);
    if (guid.isEmpty())
    {
        guid.setString(name);
    }
    auto material = Unigine::Materials::findMaterialByGUID(guid);
    if (!material)
    {
        material = Unigine::Materials::findMaterial(name);
    }
    if (!material)
    {
        return 0;
    }
    m_surfaces[surface].material = material->getGUID().getString();
    return 1;
}

void Object::setEnabled(bool enabled, int surface)
{
    m_surfaces[surface].enabled = enabled;
}

bool Object::isEnabled(int surface) const
{
    return m_surfaces[surface].enabled;
}

void Object::setMinParent(int parent, int surface)
{
    m_surfaces[surface].min_parent = parent;
}

int Object::getMinParent(int surface) const
{
    return m_surfaces[surface].min_parent;
}

void Object::setMaxParent(int parent, int surface)
{
    m_surfaces[surface].max_parent = parent;
}

int Object::getMaxParent(int surface) const
{
    return m_surfaces[surface].max_parent;
}

void Object::setCastShadow(bool enabled, int surface)
{
    m_surfaces[surface].cast_shadow = enabled;
}

bool Object::getCastShadow(int surface) const
{
    return m_surfaces[surface].cast_shadow;
}

void Object::setCastWorldShadow(bool enabled, int surface)
{
    m_surfaces[surface].cast_world_shadow = enabled;
}

bool Object::getCastWorldShadow(int surface) const
{
    return m_surfaces[surface].cast_world_shadow;
}

void Object::setBakeToEnvProbe(bool enabled, int surface)
{
    m_surfaces[surface].bake_to_env_probe = enabled;
}

bool Object::getBakeToEnvProbe(int surface) const
{
    return m_surfaces[surface].bake_to_env_probe;
}

void Object::setBakeToGI(bool enabled, int surface)
{
    m_surfaces[surface].bake_to_gi = enabled;
}

bool Object::getBakeToGI(int surface) const
{
    return m_surfaces[surface].bake_to_gi;
}

void Object::setCastEnvProbeShadow(bool enabled, int surface)
{
    m_surfaces[surface].cast_env_probe_shadow = enabled;
}

bool Object::getCastEnvProbeShadow(int surface) const
{
    return m_surfaces[surface].cast_env_probe_shadow;
}

void Object::setShadowMode(Unigine::Object::SURFACE_SHADOW_MODE mode, int surface)
{
    m_surfaces[surface].shadow_mode = +mode;
}

Unigine::Object::SURFACE_SHADOW_MODE Object::getShadowMode(int surface) const
{
    if (m_surfaces[surface].shadow_mode == +Unigine::Object::SURFACE_SHADOW_MODE_MIXED)
    {
        return Unigine::Object::SURFACE_SHADOW_MODE_MIXED;
    }
    return Unigine::Object::SURFACE_SHADOW_MODE_DYNAMIC;
}

void Object::setShadowMask(int mask, int surface)
{
    m_surfaces[surface].shadow_mask = mask;
}

int Object::getShadowMask(int surface) const
{
    return m_surfaces[surface].shadow_mask;
}

void Object::setViewportMask(int mask, int surface)
{
    m_surfaces[surface].viewport_mask = mask;
}

int Object::getViewportMask(int surface) const
{
    return m_surfaces[surface].viewport_mask;
}

void Object::setCollision(bool enabled, int surface)
{
    m_surfaces[surface].collision = enabled;
}

bool Object::getCollision(int surface) const
{
    return m_surfaces[surface].collision;
}

void Object::setCollisionMask(int mask, int surface)
{
    m_surfaces[surface].collision_mask = mask;
}

int Object::getCollisionMask(int surface) const
{
    return m_surfaces[surface].collision_mask;
}

void Object::setPhysicsIntersection(bool enabled, int surface)
{
    m_surfaces[surface].physics_intersection = enabled;
}

bool Object::getPhysicsIntersection(int surface) const
{
    return m_surfaces[surface].physics_intersection;
}

void Object::setPhysicsIntersectionMask(int mask, int surface)
{
    m_surfaces[surface].physics_intersection_mask = mask;
}

int Object::getPhysicsIntersectionMask(int surface) const
{
    return m_surfaces[surface].physics_intersection_mask;
}

void Object::setPhysicsFriction(float value, int surface)
{
    m_surfaces[surface].physics_friction = value;
}

float Object::getPhysicsFriction(int surface) const
{
    return m_surfaces[surface].physics_friction;
}

void Object::setPhysicsRestitution(float value, int surface)
{
    m_surfaces[surface].physics_restitution = value;
}

float Object::getPhysicsRestitution(int surface) const
{
    return m_surfaces[surface].physics_restitution;
}

void Object::setMinVisibleDistance(float distance, int surface)
{
    m_surfaces[surface].min_visible_distance = distance;
}

float Object::getMinVisibleDistance(int surface) const
{
    return m_surfaces[surface].min_visible_distance;
}

void Object::setMinFadeDistance(float distance, int surface)
{
    m_surfaces[surface].min_fade_distance = distance;
}

float Object::getMinFadeDistance(int surface) const
{
    return m_surfaces[surface].min_fade_distance;
}

void Object::setMaxVisibleDistance(float distance, int surface)
{
    m_surfaces[surface].max_visible_distance = distance;
}

float Object::getMaxVisibleDistance(int surface) const
{
    return m_surfaces[surface].max_visible_distance;
}

void Object::setMaxFadeDistance(float distance, int surface)
{
    m_surfaces[surface].max_fade_distance = distance;
}

float Object::getMaxFadeDistance(int surface) const
{
    return m_surfaces[surface].max_fade_distance;
}

void Object::setIntersection(bool enabled, int surface)
{
    m_surfaces[surface].intersection = enabled;
}

bool Object::getIntersection(int surface) const
{
    return m_surfaces[surface].intersection;
}

void Object::setIntersectionMask(int mask, int surface)
{
    m_surfaces[surface].intersection_mask = mask;
}

int Object::getIntersectionMask(int surface) const
{
    return m_surfaces[surface].intersection_mask;
}

// MetaUnigine extensions

Unigine::UGUID Object::getMaterialGUID(int surface) const
{
    return Unigine::UGUID(m_surfaces[surface].material.data());
}

void Object::setMaterialGUID(const Unigine::UGUID& guid, int surface)
{
    m_surfaces[surface].material = guid.getString();
}

void Object::applyUnigineObjectParams(const Unigine::ObjectPtr& object) const
{
    assert(object->getNumSurfaces() == m_surfaces.size());
    for (int surfaceIdx = 0; surfaceIdx < m_surfaces.size(); ++surfaceIdx)
    {
        copySurfaceParametersFromTo(this, object, surfaceIdx);
    }
    Node::applyUnigineNodeParams(object);
}

// ObjectMeshStatic

ObjectMeshStaticPtr ObjectMeshStatic::create()
{
    return std::make_shared<ObjectMeshStatic>();
}

ObjectMeshStaticPtr ObjectMeshStatic::create(const char* meshPath)
{
    return std::make_shared<ObjectMeshStatic>(meshPath);
}

ObjectMeshStatic::ObjectMeshStatic()
{
    m_type = Unigine::Node::TYPE::OBJECT_MESH_STATIC;
    m_typeName = "ObjectMeshStatic";
}

ObjectMeshStatic::ObjectMeshStatic(const char* meshPath) 
    : m_meshPath(meshPath)
{
    m_type = Unigine::Node::TYPE::OBJECT_MESH_STATIC;
    m_typeName = "ObjectMeshStatic";
    loadMesh();
}

Unigine::XmlPtr ObjectMeshStatic::saveXml(const Unigine::XmlPtr& xmlNode) const
{
    xmlNode->addChild(Unigine::Xml::create("mesh_name"))->setData(m_meshPath.data());
    return Object::saveXml(xmlNode);
}

bool ObjectMeshStatic::loadXml(const Unigine::XmlPtr& xml)
{
    int meshNameIdx = xml->findChild("mesh_name");
    if (meshNameIdx != -1)
    {
        m_meshPath = xml->getChild(meshNameIdx)->getData();
        loadMesh();
    }
    return Object::loadXml(xml);
}

Unigine::BoundBox ObjectMeshStatic::getBoundBox() const
{
    auto result = Object::getBoundBox();
    if (m_mesh)
    {
        Unigine::BoundBox selfBox(m_mesh->getBoundBox());
        selfBox.setTransform(getTransform());
        result.expand(selfBox);
    }
    return result;
}

UNIGINE_BOUND_BOX ObjectMeshStatic::getWorldBoundBox() const
{
    auto result = Object::getWorldBoundBox();
    if (m_mesh)
    {
        UNIGINE_BOUND_BOX selfBox(m_mesh->getBoundBox());
        selfBox.setTransform(getWorldTransform());
        result.expand(selfBox);
    }
    return result;
}

void ObjectMeshStatic::setMeshNameForce(const char* name)
{
    m_meshPath = name;
    loadMesh();
}

void ObjectMeshStatic::setMeshName(const char* name)
{
    m_meshPath = name;
}

const char* ObjectMeshStatic::getMeshName() const
{
    return m_meshPath.data();
}

bool ObjectMeshStatic::saveMesh(const char* path) const
{
    return (m_mesh && m_mesh->save(path));
}

bool ObjectMeshStatic::setMesh(const Unigine::MeshPtr& mesh, bool unique)
{
    m_surfaces.clear();
    if (m_mesh = mesh)
    {
        for (int surfaceIdx = 0; surfaceIdx < m_mesh->getNumSurfaces(); ++surfaceIdx)
        {
            m_surfaces.emplace_back().name = m_mesh->getSurfaceName(surfaceIdx);
        }
    }
    return true;
}

bool ObjectMeshStatic::getMesh(const Unigine::MeshPtr& mesh) const
{
    mesh->assignFrom(m_mesh);
    return true;
}

int ObjectMeshStatic::loadMesh()
{
    auto mesh = Unigine::Mesh::create();
    if (mesh->load(m_meshPath.data()))
    {
        return setMesh(mesh) ? 1 : 0;
    }
    return 0;
}

Unigine::ObjectMeshStaticPtr ObjectMeshStatic::createUnigineMeshStatic() const
{
    if (!m_mesh)
    {
        return Unigine::ObjectMeshStaticPtr();
    }
    auto result = Unigine::ObjectMeshStatic::create(m_mesh);
    applyUnigineObjectParams(result);
    return result;
}

bool
ObjectMeshStatic::isValid() const
{
    if (!m_mesh)
    {
        return false;
    }
    return Object::isValid();
}

Unigine::NodePtr ObjectMeshStatic::createUnigine() const
{
    return createUnigineMeshStatic();
}

// ObjectMeshSkinned

ObjectMeshSkinnedPtr ObjectMeshSkinned::create(const char* meshPath)
{
    return std::make_shared<ObjectMeshSkinned>(meshPath);
}

ObjectMeshSkinned::ObjectMeshSkinned()
{
    m_type = Unigine::Node::TYPE::OBJECT_MESH_SKINNED;
    m_typeName = "ObjectMeshSkinned";
}

ObjectMeshSkinned::ObjectMeshSkinned(const char* meshPath)
{
    m_type = Unigine::Node::TYPE::OBJECT_MESH_SKINNED;
    m_typeName = "ObjectMeshSkinned";
    setMeshNameForce(meshPath);
}

Unigine::XmlPtr ObjectMeshSkinned::saveXml(const Unigine::XmlPtr& xmlNode) const
{
    xmlNode->addChild(Unigine::Xml::create("mesh_name"))->setData(m_meshPath.data());
    if (!m_animName.empty())
    {
        xmlNode->addChild(Unigine::Xml::create("anim_name"))->setData(m_animName.data());
    }

    SAVE_CHILD_INT(xmlNode, fps_visible_camera);
    SAVE_CHILD_INT(xmlNode, fps_visible_shadow);
    SAVE_CHILD_INT(xmlNode, fps_invisible);
    SAVE_CHILD_FLOAT(xmlNode, time);
    SAVE_CHILD_FLOAT(xmlNode, speed);
    SAVE_CHILD_INT(xmlNode, playing);

    return Object::saveXml(xmlNode);
}

bool ObjectMeshSkinned::loadXml(const Unigine::XmlPtr& xml)
{
    int meshNameIdx = xml->findChild("mesh_name");
    if (meshNameIdx != -1)
    {
        setMeshNameForce(xml->getChild(meshNameIdx)->getData());
    }

    int animNameIdx = xml->findChild("anim_name");
    if (animNameIdx != -1)
    {
        setAnimName(xml->getChild(animNameIdx)->getData());
    }

    LOAD_CHILD_IF_EXISTS_INT(xml, fps_visible_camera);
    LOAD_CHILD_IF_EXISTS_INT(xml, fps_visible_shadow);
    LOAD_CHILD_IF_EXISTS_INT(xml, fps_invisible);

    LOAD_CHILD_IF_EXISTS_FLOAT(xml, time);
    LOAD_CHILD_IF_EXISTS_FLOAT(xml, speed)
    LOAD_CHILD_IF_EXISTS_INT(xml, playing);

    return Object::loadXml(xml);
}

Unigine::BoundBox ObjectMeshSkinned::getBoundBox() const
{
    auto result = Object::getBoundBox();
    if (m_mesh)
    {
        Unigine::BoundBox selfBox(m_mesh->getBoundBox());
        selfBox.setTransform(getTransform());
        result.expand(selfBox);
    }
    return result;
}

UNIGINE_BOUND_BOX ObjectMeshSkinned::getWorldBoundBox() const
{
    auto result = Object::getWorldBoundBox();
    if (m_mesh)
    {
        UNIGINE_BOUND_BOX selfBox(m_mesh->getBoundBox());
        selfBox.setTransform(getWorldTransform());
        result.expand(selfBox);
    }
    return result;
}

const char*
ObjectMeshSkinned::getMeshName() const
{
    return m_meshPath.data();
}

void
ObjectMeshSkinned::setMeshNameForce(const char* path)
{
    m_surfaces.clear();
    m_animations.clear();
    m_animLayers.clear();

    m_mesh = Unigine::Mesh::create();
    if (!m_mesh->load(path))
    {
        m_mesh.clear();
        return;
    }

    for (int surfaceIdx = 0; surfaceIdx < m_mesh->getNumSurfaces(); ++surfaceIdx)
    {
        auto& surface = m_surfaces.emplace_back();
        surface.name = m_mesh->getSurfaceName(surfaceIdx);
        surface.material = Unigine::Materials::getMaterial(0)->getGUID().getString();
    }

    int numAnimations = m_mesh->getNumAnimations();
    for (int animIdx = 0; animIdx < numAnimations; ++animIdx)
    {
        m_animations.emplace_back(AnimInfo{m_mesh->getAnimationName(animIdx), m_mesh, animIdx});
    }

    m_animLayers.emplace_back(AnimLayerInfo{0, true, 1.0f});

    m_meshPath = path;
}

int
ObjectMeshSkinned::setMesh(const Unigine::MeshPtr& mesh)
{
    m_surfaces.clear();
    m_animations.clear();
    m_animLayers.clear();
    m_mesh = mesh;
    if (!m_mesh)
    {
        return 1;
    }

    Unigine::UGUID defaultMatGuid;
    if (auto defMat = Unigine::Materials::findMaterial("mesh_base"))
    {
        defaultMatGuid = defMat->getGUID();
    }
    for (int surfaceIdx = 0; surfaceIdx < m_mesh->getNumSurfaces(); ++surfaceIdx)
    {
        auto& surface = m_surfaces.emplace_back();
        surface.name = m_mesh->getSurfaceName(surfaceIdx);
        surface.material = defaultMatGuid.getString();
    }

    int numAnimations = m_mesh->getNumAnimations();
    for (int animIdx = 0; animIdx < numAnimations; ++animIdx)
    {
        m_animations.emplace_back(AnimInfo{m_mesh->getAnimationName(animIdx), m_mesh, animIdx});
    }

    m_animLayers.emplace_back(AnimLayerInfo{0, true, 1.0f});
    return 1;
}

int
ObjectMeshSkinned::setMeshKeepMaterials(const Unigine::MeshPtr& mesh)
{
    if (!mesh || (mesh->getNumSurfaces() != m_surfaces.size()))
    {
        return 0;
    }

    m_animations.clear();
    m_animLayers.clear();
    m_mesh = mesh;

    int numAnimations = m_mesh->getNumAnimations();
    for (int animIdx = 0; animIdx < numAnimations; ++animIdx)
    {
        m_animations.emplace_back(AnimInfo{m_mesh->getAnimationName(animIdx), m_mesh, animIdx});
    }

    m_animLayers.emplace_back(AnimLayerInfo{0, true, 1.0f});
    return 1;
}

int
ObjectMeshSkinned::getMesh(Unigine::MeshPtr& mesh) const
{
    if (!m_mesh)
    {
        return 0;
    }
    mesh->assignFrom(m_mesh);
    return 1;
}

int
ObjectMeshSkinned::getNumBones() const
{
    if (!m_mesh)
    {
        return 0;
    }
    return m_mesh->getNumBones();
}

const char*
ObjectMeshSkinned::getBoneName(int bone) const
{
    if (!m_mesh)
    {
        return nullptr;
    }
    return m_mesh->getBoneName(bone);
}

int
ObjectMeshSkinned::findBone(const char* name) const
{
    if (!m_mesh)
    {
        return -1;
    }
    return m_mesh->findBone(name);
}

const char*
ObjectMeshSkinned::getAnimName() const
{
    return m_animName.data();
}

void
ObjectMeshSkinned::setAnimName(const char* name)
{
    int animIdx = findAnimation(name);
    if (animIdx < 0)
    {
        animIdx = addAnimation(name);
    }
    if (animIdx < 0)
    {
        return;
    }

    setNumLayers(1);
    setLayer(0, true, 1.0f);
    setAnimation(0, animIdx);
    m_animName = name;
}

int
ObjectMeshSkinned::getNumAnimations() const
{
    return int(m_animations.size());
}

int
ObjectMeshSkinned::findAnimation(const char* path) const
{
    for (int animIdx = 0; animIdx < m_animations.size(); ++animIdx)
    {
        if (m_animations[animIdx].path == path)
        {
            return animIdx;
        }
    }
    return -1;
}

int
ObjectMeshSkinned::addAnimation(const char* path)
{
    auto anim = Unigine::Mesh::create();
    if (!anim->load(path))
    {
        return -1;
    }
    for (int animIdx = 0; animIdx < anim->getNumAnimations(); ++animIdx)
    {
        m_animations.emplace_back(AnimInfo{path, anim, animIdx});
    }
    return int(m_animations.size() - 1);
}

int
ObjectMeshSkinned::getNumAnimationFrames(int num) const
{
    assert(m_animations.size() > num);

    const auto& animInfo = m_animations[num];
    return animInfo.anim->getNumAnimationFrames(animInfo.animIdx);
}

int
ObjectMeshSkinned::addLayer()
{
    m_animLayers.emplace_back();
    return int(m_animLayers.size() - 1);
}

void
ObjectMeshSkinned::removeLayer(int layer)
{
    assert(layer < m_animLayers.size());

    m_animLayers.erase(m_animLayers.begin() + layer);
}

void
ObjectMeshSkinned::setNumLayers(int layers)
{
    m_animLayers.resize(layers);
}

int ObjectMeshSkinned::getNumLayers() const
{
    return int(m_animLayers.size());
}

void
ObjectMeshSkinned::setLayer(int layer, bool enabled, float weight)
{
    assert(layer < m_animLayers.size());

    auto& layerInfo = m_animLayers[layer];
    layerInfo.enabled = enabled;
    layerInfo.weight = weight;
}

void
ObjectMeshSkinned::setLayerEnabled(int layer, bool enabled)
{
    assert(layer < m_animLayers.size());

    auto& layerInfo = m_animLayers[layer];
    layerInfo.enabled = enabled;
}

bool
ObjectMeshSkinned::isLayerEnabled(int layer) const
{
    assert(layer < m_animLayers.size());

    const auto& layerInfo = m_animLayers[layer];
    return layerInfo.enabled;
}

void
ObjectMeshSkinned::setLayerWeight(int layer, float weight)
{
    assert(layer < m_animLayers.size());

    auto& layerInfo = m_animLayers[layer];
    layerInfo.weight = weight;
}

float
ObjectMeshSkinned::getLayerWeight(int layer) const
{
    assert(layer < m_animLayers.size());

    const auto& layerInfo = m_animLayers[layer];
    return layerInfo.weight;
}

int
ObjectMeshSkinned::setAnimation(int layer, int animation)
{
    if (layer >= m_animLayers.size())
    {
        return -1;
    }

    auto& layerInfo = m_animLayers[layer];
    layerInfo.animIdx = animation;
    return 1;
}

int
ObjectMeshSkinned::getAnimation(int layer) const
{
    assert(layer < m_animLayers.size());

    const auto& layerInfo = m_animLayers[layer];
    return layerInfo.animIdx;
}

Unigine::Math::mat4
ObjectMeshSkinned::getBoneBindTransform(int bone) const
{
    if (!m_mesh)
    {
        return Unigine::Math::mat4::IDENTITY;
    }
    return m_mesh->getBoneTransform(bone);
}

Unigine::Math::mat4
ObjectMeshSkinned::getBoneLayerTransform(int layer, int bone) const
{
    if (!m_mesh)
    {
        return Unigine::Math::mat4::IDENTITY;
    }

    if (layer >= m_animLayers.size())
    {
        return Unigine::Math::mat4::IDENTITY;
    }

    const auto& layerInfo = m_animLayers[layer];
    const auto& animInfo = m_animations[layerInfo.animIdx];
    const auto& animData = animInfo.anim;
    const auto& animIdx = animInfo.animIdx;

    int animFramesCount = animData->getNumAnimationFrames(animIdx);

    Unigine::Vector<short> animBones;
    animData->getAnimationBones(animIdx, animBones);

    float floatFrameIdx = 0.0f;
    float frameOffset = std::modf(m_time, &floatFrameIdx);

    int frameIdx(std::lround(floatFrameIdx));
    if (frameIdx >= animFramesCount)
    {
        frameIdx = animFramesCount - 1;
    }

    Unigine::Vector<Unigine::Math::mat4> boneTransforms(animBones.size());
    Unigine::Vector<Unigine::Math::mat4> boneTransformsNextFrame;
    animData->getBoneTransforms(boneTransforms, animIdx, frameIdx);

    // In case time is between frames (frame is not integer) and there is next frame available
    // load next frame transforms from animation to use for interpolaton
    if ((frameOffset > std::numeric_limits<float>::lowest()) && (frameIdx + 1 < animFramesCount))
    {
        boneTransformsNextFrame.resize(animBones.size());
        animData->getBoneTransforms(boneTransformsNextFrame, animIdx, frameIdx + 1);
    }

    auto getBoneTransformLamda =
        [this, &animBones, &boneTransforms, &boneTransformsNextFrame, frameOffset](int bone)
    {
        auto animBoneIt = std::find(animBones.begin(), animBones.end(), bone);
        if (animBoneIt == animBones.end())
        {
            return m_mesh->getBoneTransform(bone);
        }
        int animBoneIdx = animBoneIt - animBones.begin();
        // If we have next frame transforms, use them for interpolation between frames
        if (boneTransformsNextFrame.size() == boneTransforms.size())
        {
            return Unigine::Math::lerp(boneTransforms[animBoneIdx],
                boneTransformsNextFrame[animBoneIdx],
                frameOffset);
        }
        return boneTransforms[bone];
    };

    auto result = getBoneTransformLamda(bone);

    int boneParent = m_mesh->getBoneParent(bone);
    while (boneParent >= 0)
    {
        result *= getBoneTransformLamda(boneParent);
        boneParent = m_mesh->getBoneParent(boneParent);
    }
    return result;
}

Unigine::Math::mat4
ObjectMeshSkinned::getBoneTransform(int bone) const
{
    if (!m_mesh)
    {
        return Unigine::Math::mat4::IDENTITY;
    }

    Unigine::Math::mat4 result = m_mesh->getBoneTransform(bone);
    for (int layerIdx = 0; layerIdx < m_animLayers.size(); ++layerIdx)
    {
        const auto& layer = m_animLayers[layerIdx];
        if (!layer.enabled)
        {
            continue;
        }
        const auto& layerTransform = getBoneLayerTransform(layerIdx, bone);
        result = Unigine::Math::lerp(result, layerTransform, layer.weight);
    }
    return result;
}

Unigine::Math::Mat4
ObjectMeshSkinned::getBoneWorldTransform(int bone) const
{
    return getWorldTransform() * Unigine::Math::Mat4(getBoneTransform(bone));
}

void
ObjectMeshSkinned::setTime(float time)
{
    m_time = time;
}

float
ObjectMeshSkinned::getTime() const
{
    return m_time;
}

void
ObjectMeshSkinned::setSpeed(float speed)
{
    m_speed = speed;
}

float
ObjectMeshSkinned::getSpeed() const
{
    return m_speed;
}

void
ObjectMeshSkinned::play()
{
    m_playing = true;
}

void
ObjectMeshSkinned::stop()
{
    m_playing = false;
}

bool
ObjectMeshSkinned::isPlaying() const
{
    return m_playing;
}

bool
ObjectMeshSkinned::isStopped() const
{
    return !m_playing;
}

Unigine::ObjectMeshSkinnedPtr ObjectMeshSkinned::createUnigineMeshSkinned() const
{
    if (!m_mesh)
    {
        return Unigine::ObjectMeshSkinnedPtr();
    }
    auto result = Unigine::ObjectMeshSkinned::create(m_mesh);
    applyUnigineObjectParams(result);

    if (!m_animName.empty())
    {
        result->setAnimNameForce(m_animName.data());
    }

    for (const auto& anim : m_animations)
    {
        if (result->findAnimation(anim.path.data()) == -1)
        {
            result->addAnimation(anim.path.data());
        }
    }

    result->setNumLayers(m_animLayers.size());
    for (int layerIdx = 0; layerIdx < m_animLayers.size(); ++layerIdx)
    {
        const auto& layer = m_animLayers[layerIdx];
        const auto& anim = m_animations[layer.animIdx];
        int resultAnimIdx = result->findAnimation(anim.path.data());
        if (resultAnimIdx != -1)
        {
            result->setAnimation(layerIdx, resultAnimIdx);
        }
        result->setLayer(layerIdx, layer.enabled, layer.weight);
    }

    result->setTime(m_time);
    result->setSpeed(m_speed);
    if (m_playing)
    {
        result->play();
    }

    return result;
}

bool
ObjectMeshSkinned::isValid() const
{
    if (!m_mesh)
    {
        return false;
    }
    return Object::isValid();
}

Unigine::NodePtr ObjectMeshSkinned::createUnigine() const
{
    return createUnigineMeshSkinned();
}

// ObjectMeshCluster

ObjectMeshClusterPtr ObjectMeshCluster::create(const char* meshPath)
{
    return std::make_shared<ObjectMeshCluster>(meshPath);
}

ObjectMeshCluster::ObjectMeshCluster()
{
    m_type = Unigine::Node::TYPE::OBJECT_MESH_CLUSTER;
    m_typeName = "ObjectMeshCluster";
}

ObjectMeshCluster::ObjectMeshCluster(const char* meshPath)
    : m_meshPath(meshPath)
{
    m_type = Unigine::Node::TYPE::OBJECT_MESH_CLUSTER;
    m_typeName = "ObjectMeshCluster";
    loadMesh();
}

Unigine::XmlPtr ObjectMeshCluster::saveXml(const Unigine::XmlPtr& xmlNode) const
{
    xmlNode->addChild(Unigine::Xml::create("mesh_name"))->setData(m_meshPath.data());

    for (const auto& transform : m_transforms)
    {
        xmlNode->addChild("position")->setData(mat4ToString(transform));
    }

    return Object::saveXml(xmlNode);
}

bool ObjectMeshCluster::loadXml(const Unigine::XmlPtr& xml)
{
    int meshNameIdx = xml->findChild("mesh_name");
    if (meshNameIdx != -1)
    {
        m_meshPath = xml->getChild(meshNameIdx)->getData();
        loadMesh();
    }

    m_transforms.clear();
    for (int childIdx = 0; childIdx < xml->getNumChildren(); ++childIdx)
    {
        auto xmlChild = xml->getChild(childIdx);
        if (!strcmp(xmlChild->getName(), "position"))
        {
            m_transforms.push_back(Unigine::Math::mat4(xmlToMat4(xmlChild)));
        }
    }

    return Object::loadXml(xml);
}

void ObjectMeshCluster::setMeshName(const char* name)
{
    m_meshPath = name;
}

const char* ObjectMeshCluster::getMeshName() const
{
    return m_meshPath.data();
}

void ObjectMeshCluster::setVisibleDistance(float distance)
{
    m_visibleDistance = distance;
}

float ObjectMeshCluster::getVisibleDistance() const
{
    return m_visibleDistance;
}

int ObjectMeshCluster::addMeshTransform()
{
    m_transforms.emplace_back();
    return m_transforms.size() - 1;
}

Unigine::Math::mat4 ObjectMeshCluster::getMeshTransform(int num) const
{
    return m_transforms[num];
}

void ObjectMeshCluster::setMeshTransform(int num, const Unigine::Math::mat4& transform)
{
    m_transforms[num] = transform;
}

void ObjectMeshCluster::clearMeshes()
{
    m_transforms.clear();
}

int ObjectMeshCluster::loadMesh()
{
    m_surfaces.clear();

    m_mesh = Unigine::Mesh::create();
    if (m_mesh->load(m_meshPath.data()))
    {
        for (int surfaceIdx = 0; surfaceIdx < m_mesh->getNumSurfaces(); ++surfaceIdx)
        {
            m_surfaces.emplace_back().name = m_mesh->getSurfaceName(surfaceIdx);
        }
        return 1;
    }

    m_mesh.clear();
    return 0;
}

Unigine::ObjectMeshClusterPtr ObjectMeshCluster::createUnigineCluster() const
{
    auto result = Unigine::ObjectMeshCluster::create(m_meshPath.data());
    result->setVisibleDistance(getVisibleDistance());
    for (const auto& transform : m_transforms)
    {
        int transformIdx = result->addMeshTransform();
        result->setMeshTransform(transformIdx, transform);
    }
    applyUnigineObjectParams(result);
    return result;
}

bool
ObjectMeshCluster::isValid() const
{
    if (!m_mesh)
    {
        return false;
    }
    return Object::isValid();
}

Unigine::NodePtr ObjectMeshCluster::createUnigine() const
{
    return createUnigineCluster();
}

}//namespace MetaUnigine
