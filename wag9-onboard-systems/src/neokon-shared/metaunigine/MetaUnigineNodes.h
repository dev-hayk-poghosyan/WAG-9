#pragma once

#include <UnigineXml.h>
#include <UnigineMathLib.h>
#include <UnigineComponentSystem.h>

#include <string>
#include <memory>
#include <vector>
#include <optional>

namespace MetaUnigine
{

class Node;
using NodePtr = std::shared_ptr<Node>;

class NodeDummy;
using NodeDummyPtr = std::shared_ptr<NodeDummy>;

class NodeReference;
using NodeReferencePtr = std::shared_ptr<NodeReference>;

class WorldTransformBone;
using WorldTransformBonePtr = std::shared_ptr<WorldTransformBone>;

/**
 * @see Unigine::Node documentation.
 * Limitation/difference with Unigine's: adding/getting components via 'ComponentSystem::'
 * is not supported. Use 'Node::getComponent/addComponent' functions only.
 */
class Node : public std::enable_shared_from_this<Node>
{
public:
    Node();
    virtual ~Node() = default;

    Unigine::Node::TYPE getType() const;
    const char* getTypeName() const;
    virtual Unigine::XmlPtr saveXml(const Unigine::XmlPtr& xml) const;
    virtual bool loadXml(const Unigine::XmlPtr& xml);
    virtual Unigine::BoundBox getBoundBox() const;
    virtual UNIGINE_BOUND_BOX getWorldBoundBox() const;

    bool saveWorld(const Unigine::XmlPtr& xml) const;

    void setEnabled(bool enabled);
    bool isEnabled() const;

    void setName(const char* name);
    const char* getName() const;

    NodePtr getParent() const;
    void setParent(const NodePtr& parent);
    void addChild(const NodePtr& child);

    int getNumChildren() const;
    int findChild(const char* name) const;
    NodePtr getChild(int num) const;
    NodePtr findNode(const char* name, int recursive = 0) const;

    const Unigine::Math::Mat4& getTransform() const;
    void setTransform(const Unigine::Math::Mat4& tranform);

    Unigine::Math::Vec3 getPosition() const;
    void setPosition(const Unigine::Math::Vec3& pos);

    void setScale(const Unigine::Math::vec3& s);
    Unigine::Math::vec3 getScale() const;

    /**
     * Sets the node rotation:
     * If @p identity is 'true', the node's current scale should be dropped (faster),
     * if 'false', the scale should be taken into account.
     */
    void setRotation(const Unigine::Math::quat& rot, bool identity = false);
    Unigine::Math::quat getRotation() const;

    Unigine::Math::vec3 getDirection(Unigine::MathLib::AXIS axis = Unigine::Math::AXIS_NZ) const;

    void setWorldTransform(const Unigine::Math::Mat4& transform);
    Unigine::Math::Mat4 getWorldTransform() const;
    void setWorldPosition(const Unigine::Math::Vec3& pos);
    Unigine::Math::Vec3 getWorldPosition() const;
    void setWorldScale(const Unigine::Math::vec3& s);
    Unigine::Math::vec3 getWorldScale() const;
    void setWorldRotation(const Unigine::Math::quat& rot, bool identity = false);
    Unigine::Math::quat getWorldRotation() const;
    Unigine::Math::vec3 getWorldDirection(Unigine::MathLib::AXIS axis = Unigine::Math::AXIS_NZ) const;

    int findProperty(const char* name) const;
    int addProperty(const char* name);
    Unigine::PropertyPtr getProperty(int num = 0) const;

public: // Extensions
    virtual bool isValid() const;
    virtual Unigine::NodePtr createUnigine() const;
    Unigine::NodePtr createUnigineTree() const;

protected:
    void applyUnigineNodeParams(const Unigine::NodePtr& node) const;
    bool addComponentInternal(const std::shared_ptr<Unigine::ComponentBase>& component);

public:
    template <class C>
    C* getComponent()
    {
        for (auto& component : m_components)
        {
            C* casted = dynamic_cast<C*>(component.get());
            if (casted)
            {
                return casted;
            }
        }
        int propIdx = findProperty(C::getPropertyName());
        if (propIdx < 0)
        {
            return nullptr;
        }
        auto result = std::make_shared<C>(Unigine::NodePtr(), 0);
        result->property = getProperty(propIdx);
        if (!addComponentInternal(result))
        {
            return nullptr;
        }
        return result.get();
    }

    template <class C>
    C* addComponent()
    {
        int propIdx = addProperty(C::getPropertyName());
        if (propIdx < 0)
        {
            return nullptr;
        }
        auto result = std::make_shared<C>(Unigine::NodePtr(), 0);
        result->property = getProperty(propIdx);
        if (!addComponentInternal(result))
        {
            return nullptr;
        }
        return result.get();
    }

protected:
    Unigine::Node::TYPE m_type = Unigine::Node::TYPE::ANY_TYPE;
    const char* m_typeName = nullptr;

    int m_id;
    bool m_enabled = true;
    std::string m_name;

    Unigine::Math::Mat4 m_transform;

    std::weak_ptr<Node> m_parent;
    std::vector<NodePtr> m_children;

    std::vector<Unigine::PropertyPtr> m_properties;
    std::vector<std::shared_ptr<Unigine::ComponentBase>> m_components;
};

/**
 * @see Unigine::NodeDummy documentation.
 */
class NodeDummy : public Node
{
public:
    static NodeDummyPtr create();
    NodeDummy();

public: // Extensions
    Unigine::NodeDummyPtr createUnigineDummy() const;
    virtual Unigine::NodePtr createUnigine() const;
};

/**
 * @see Unigine::NodeReference documentation.
 */
class NodeReference : public Node
{
public:
    static NodeReferencePtr create(const char* name);
    NodeReference(const char* name);
    NodeReference();

    virtual Unigine::XmlPtr saveXml(const Unigine::XmlPtr& xml) const;
    virtual bool loadXml(const Unigine::XmlPtr& xml);

    std::shared_ptr<Node> getReference() const;

public: // Extensions
    virtual Unigine::NodePtr createUnigine() const;

protected:
    std::string m_refName;
};

/**
 * @see Unigine::WorldTransformBone documentation.
 */
class WorldTransformBone : public Node
{
public:
    static WorldTransformBonePtr create(const char* name);
    WorldTransformBone(const char* name);
    WorldTransformBone();

    virtual Unigine::XmlPtr saveXml(const Unigine::XmlPtr& xml) const;
    virtual bool loadXml(const Unigine::XmlPtr& xml);

public: // Extensions
    virtual Unigine::NodePtr createUnigine() const;

protected:
    std::string m_boneName;
};

/**
 * Node used by MetaUnigine loadNode/saveNode to handle unsupported types
 */
class NodeNotSupported : public Node
{
public:
    NodeNotSupported(){};

    virtual Unigine::XmlPtr saveXml(const Unigine::XmlPtr& xml) const;
    virtual bool loadXml(const Unigine::XmlPtr& xml);

public: // Extensions
    virtual Unigine::NodePtr createUnigine() const;

protected:
    Unigine::XmlPtr m_xmlData;
};

}//namespace MetaUnigine
