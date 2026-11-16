#include "MetaUnigineNodes.h"
#include "MetaUnigineObjects.h"
#include "MetaUnigineUtils.h"
#include "MetaUnigine.h"

#include <mutex>
#include <random>
#include <unordered_set>
#include <sstream>

#include <UnigineString.h>
#include <UnigineWorlds.h>
using namespace Unigine;

namespace MetaUnigine
{

static std::mutex id_lock;
static uint64_t global_seed = std::random_device()();
static std::unordered_set<int> instances_id;

Node::Node()
{
    m_typeName = "Node";
    id_lock.lock();
    do
    {
        global_seed = global_seed * 1664525 + 1013904223;
        m_id = int(global_seed % INT_MAX) & 0x7FFFFFFF;
    } while (instances_id.find(m_id) != instances_id.end());
    instances_id.emplace(m_id);
    id_lock.unlock();
}

Unigine::Node::TYPE Node::getType() const
{
    return m_type;
}

const char* Node::getTypeName() const
{
    return m_typeName;
}

XmlPtr Node::saveXml(const XmlPtr& xmlNode) const
{
    xmlNode->setArg("type", m_typeName);
    xmlNode->setArg("id", std::to_string(m_id).data());
    if (!m_name.empty())
    {
        xmlNode->setArg("name", m_name.data());
    }

    if (!m_enabled)
    {
        xmlNode->setArg("enabled", "0");
    }

    if(!m_properties.empty())
    {
        auto xmlProps = xmlNode->addChild("properties");
        for (const auto& prop : m_properties)
        {
            auto xmlProp = xmlProps->addChild("property", String::format("parent=\"%s\" guid=\"%s\"",
                prop->getParent()->getGUID().getString(), prop->getGUID().getString()));
            prop->saveWorld(xmlProp);
        }
    }

    xmlNode->addChild("transform")->setData(mat4ToString(m_transform));

    return xmlNode;
}

bool Node::loadXml(const Unigine::XmlPtr& xml)
{
    int idIdx = xml->findArg("id");
    if (idIdx != -1)
    {
        m_id = std::atoi(xml->getArgValue(idIdx));
    }

    int nameIdx = xml->findArg("name");
    if (nameIdx != -1)
    {
        m_name = xml->getArgValue(nameIdx);
    }

    int enabledIdx = xml->findArg("enabled");
    if (enabledIdx != -1)
    {
        m_enabled = (std::atoi(xml->getArgValue(enabledIdx)) == 1);
    }

    if (auto xmlTransform = xml->getChild("transform"))
    {
        m_transform = xmlToMat4(xmlTransform);
    }

    m_properties.clear();
    int propsIdx = xml->findChild("properties");
    if (propsIdx != -1)
    {
        auto xmlProps = xml->getChild(propsIdx);
        for (int propIdx = 0; propIdx < xmlProps->getNumChildren(); ++propIdx)
        {
            auto xmlProp = xml->getChild(propIdx);
            auto prop = Property::create();
            if (prop->loadWorld(xmlProp))
            {
                m_properties.push_back(prop);
            }
        }
    }

    return true;
}

bool Node::saveWorld(const XmlPtr& xml) const
{
    auto selfXml = xml->addChild(saveXml(Xml::create("node")));
    for (const auto& child : m_children)
    {
        if (!child->saveWorld(selfXml))
        {
            return false;
        }
    }
    return true;
}

void Node::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

bool Node::isEnabled() const
{
    return true;
}

void Node::setName(const char* name)
{
    m_name = name;
}

const char* Node::getName() const
{
    return m_name.data();
}

NodePtr Node::getParent() const
{
    return m_parent.lock();
}

void Node::setParent(const NodePtr& newParent)
{
    // Also protect from deleting while erasing from previous parent - before adding to the new one
    auto sharedThis = shared_from_this();

    auto oldParent = m_parent.lock();
    if (oldParent)
    {
        auto& children = oldParent->m_children;
        auto childIt = std::find(children.begin(), children.end(), sharedThis);
        if (childIt != children.end())
        {
            children.erase(childIt);
        }
        m_parent.reset();
    }

    if (newParent)
    {
        m_parent = newParent;

        auto& children = newParent->m_children;
        children.emplace_back(sharedThis);
    }
}

void Node::addChild(const NodePtr& child)
{
    child->setParent(shared_from_this());
}

int Node::getNumChildren() const
{
    return m_children.size();
}

int Node::findChild(const char* name) const
{
    for (int childIndex = 0; childIndex < m_children.size(); ++childIndex)
    {
        const auto& child = m_children[childIndex];
        if (child->m_name == name)
        {
            return childIndex;
        }
    }
    return -1;
}

NodePtr Node::getChild(int num) const
{
    return m_children[num];
}

NodePtr Node::findNode(const char* name, int recursive) const
{
    for (int childIndex = 0; childIndex < m_children.size(); ++childIndex)
    {
        const auto& child = m_children[childIndex];
        if (child->m_name == name)
        {
            return child;
        }
    }
    if (recursive)
    {
        for (int childIndex = 0; childIndex < m_children.size(); ++childIndex)
        {
            const auto& child = m_children[childIndex];
            if (auto found = child->findNode(name, recursive))
            {
                return found;
            }
        }
    }
    return NodePtr();
}

const Math::Mat4& Node::getTransform() const
{
    return m_transform;
}

void Node::setTransform(const Math::Mat4& tranform)
{
    m_transform = tranform;
}

Math::Vec3 Node::getPosition() const
{
    return getTransform().getTranslate();
}

void Node::setPosition(const Math::Vec3& pos)
{
    Math::Mat4 transform = getTransform();
    setTransform(Math::Mat4(transform.getRotate(), pos));
}

void Node::setScale(const Math::vec3& s)
{
    Math::Mat4 transform = getTransform();
    Math::Mat4 scaleMat; scaleMat.setScale(Math::Vec3(transform.getScale()));
    setWorldTransform(Math::Mat4(transform.getRotate(), transform.getTranslate()) * scaleMat);
}

Math::vec3 Node::getScale() const
{
    return getTransform().getScale();
}

void Node::setRotation(const Math::quat& rot, bool identity)
{
    Math::Mat4 transform = getTransform();
    if (!identity)
    {
        Math::Mat4 scaleMat; scaleMat.setScale(Math::Vec3(transform.getScale()));
        setTransform(Math::Mat4(rot, transform.getTranslate()) * scaleMat);
    }
    setTransform(Math::Mat4(rot, transform.getTranslate()));
}

Math::quat Node::getRotation() const
{
    return getTransform().getRotate();
}

Math::vec3 Node::getDirection(MathLib::AXIS axis) const
{
    Math::vec3 axisVecs[] = {
            Math::vec3_right,
            Math::vec3_forward,
            Math::vec3_up,
            Math::vec3_left,
            Math::vec3_back,
            Math::vec3_down
    };
    return getRotation() * axisVecs[axis];
}

void Node::setWorldTransform(const Math::Mat4& transform)
{
    if (auto parent = getParent())
    {
        m_transform = transform * Math::inverse(getParent()->getWorldTransform());
    }
    else
    {
        m_transform = transform;
    }
}

Math::Mat4 Node::getWorldTransform() const
{
    Math::Mat4 worldTransform;

    auto node = shared_from_this();
    while (node)
    {
        worldTransform = worldTransform * node->getTransform();
        node = node->getParent();
    }

    return worldTransform;
}

void Node::setWorldPosition(const Math::Vec3& pos)
{
    Math::Mat4 worldTransform = getWorldTransform();
    setWorldTransform(Math::Mat4(worldTransform.getRotate(), pos));
}

Math::Vec3 Node::getWorldPosition() const
{
    return getWorldTransform().getTranslate();
}

void Node::setWorldScale(const Math::vec3& s)
{
    Math::Mat4 worldTransform = getWorldTransform();
    Math::Mat4 scaleMat; scaleMat.setScale(Math::Vec3(worldTransform.getScale()));
    setWorldTransform(Math::Mat4(worldTransform.getRotate(), worldTransform.getTranslate()) * scaleMat);
}

Math::vec3 Node::getWorldScale() const
{
    return getWorldTransform().getScale();
}

void Node::setWorldRotation(const Math::quat& rot, bool identity)
{
    Math::Mat4 worldTransform = getWorldTransform();
    if (!identity)
    {
        Math::Mat4 scaleMat; scaleMat.setScale(Math::Vec3(worldTransform.getScale()));
        setTransform(Math::Mat4(rot, worldTransform.getTranslate()) * scaleMat);
    }
    setWorldTransform(Math::Mat4(rot, worldTransform.getTranslate()));
}

Math::quat Node::getWorldRotation() const
{
    return getWorldTransform().getRotate();
}

Math::vec3 Node::getWorldDirection(MathLib::AXIS axis) const
{
    Math::vec3 axisVecs[] = {
            Math::vec3_right,
            Math::vec3_forward,
            Math::vec3_up,
            Math::vec3_left,
            Math::vec3_back,
            Math::vec3_down
    };
    return getWorldRotation()* axisVecs[axis];
}

int Node::findProperty(const char* name) const
{
    for (int propIndex = 0; propIndex < m_properties.size(); ++propIndex)
    {
        const auto& prop = m_properties[propIndex];
        if (!strcmp(prop->getName(), name))
        {
            return propIndex;
        }
    }
    return -1;
}

int Node::addProperty(const char* name)
{
    auto prop = Properties::findProperty(name);
    if (!prop)
    {
        return -1;
    }
    m_properties.push_back(prop->inherit());
    return m_properties.size() - 1;
}

PropertyPtr Node::getProperty(int num) const
{
    return m_properties[num];
}

BoundBox Node::getBoundBox() const
{
    BoundBox result;
    for (const auto& child : m_children)
    {
        result.expand(child->getBoundBox());
    }
    return result;
}

UNIGINE_BOUND_BOX Node::getWorldBoundBox() const
{
    UNIGINE_BOUND_BOX result;
    for (const auto& child : m_children)
    {
        result.expand(child->getWorldBoundBox());
    }
    return result;
}

bool
Node::isValid() const
{
    for (const auto& child : m_children)
    {
        if (!child->isValid())
        {
            return false;
        }
    }
    return true;
}

Unigine::NodePtr Node::createUnigine() const
{
    Log::error("MetaUnigine::createUnigine(): \"%s\" type not supported", m_typeName);
    return {};
}

Unigine::NodePtr Node::createUnigineTree() const
{
    auto result = createUnigine();
    if (!result)
    {
        return {};
    }
    for (const auto& child : m_children)
    {
        auto unigineChild = child->createUnigineTree();
        assert(unigineChild != nullptr);
        result->addChild(unigineChild);
    }
    return result;
}

void Node::applyUnigineNodeParams(const Unigine::NodePtr& node) const
{
    node->setName(m_name.data());
    node->setTransform(getTransform());

    for (const auto& prop : m_properties)
    {
        node->addProperty(prop->clone());
    }
}

bool
Node::addComponentInternal(const std::shared_ptr<ComponentBase>& component)
{
    // Manually reconnect 'component->variables' to property's params
    auto& vars = component->variables;
    for (int varIdx = 0; varIdx < vars.size(); varIdx++)
    {
        auto& var = vars[varIdx];
        if (auto param = component->property->getParameterPtr(var->getName()))
        {
            var->setParameter(param);
        }
    }

    // Manually reconnecting 'component->structs' to property's params is not required due to they
    // are also added to 'component->variables'

    m_components.push_back(component);
    return true;
}

// NodeDummy

NodeDummyPtr NodeDummy::create()
{
    return std::make_shared<NodeDummy>();
}

NodeDummy::NodeDummy()
{
    m_type = Unigine::Node::TYPE::NODE_DUMMY;
    m_typeName = "NodeDummy";
}

Unigine::NodeDummyPtr NodeDummy::createUnigineDummy() const
{
    auto result = Unigine::NodeDummy::create();
    applyUnigineNodeParams(result);
    return result;
}

Unigine::NodePtr NodeDummy::createUnigine() const
{
    return createUnigineDummy();
}

// NodeReference

NodeReferencePtr
NodeReference::create(const char* name)
{
    return std::make_shared<NodeReference>(name);
}

NodeReference::NodeReference(const char* name)
{
    m_type = Unigine::Node::TYPE::NODE_REFERENCE;
    m_typeName = "NodeReference";
    m_refName = name;
}

NodeReference::NodeReference()
{
    m_type = Unigine::Node::TYPE::NODE_REFERENCE;
    m_typeName = "NodeReference";
}

Unigine::XmlPtr NodeReference::saveXml(const Unigine::XmlPtr& xmlNode) const
{
    xmlNode->addChild(Unigine::Xml::create("reference"))->setData(m_refName.data());
    return Node::saveXml(xmlNode);
}

bool NodeReference::loadXml(const Unigine::XmlPtr& xml)
{
    int referenceIdx = xml->findChild("reference");
    if (referenceIdx != -1)
    {
        m_refName = xml->getChild(referenceIdx)->getData();
    }
    return Node::loadXml(xml);
}

std::shared_ptr<Node>
NodeReference::getReference() const
{
    return MetaUnigine::loadNode(m_refName.data(), true);
}

Unigine::NodePtr
NodeReference::createUnigine() const
{
    auto result = Unigine::NodeReference::create(m_refName.data());
    applyUnigineNodeParams(result);
    return result;
}

// WorldTransformBone

WorldTransformBonePtr
WorldTransformBone::create(const char* name)
{
    return std::make_shared<WorldTransformBone>(name);
}

WorldTransformBone::WorldTransformBone(const char* name)
{
    m_type = Unigine::Node::TYPE::WORLD_TRANSFORM_BONE;
    m_typeName = "WorldTransformBone";
    m_boneName = name;
}

WorldTransformBone::WorldTransformBone()
{
    m_type = Unigine::Node::TYPE::WORLD_TRANSFORM_BONE;
    m_typeName = "WorldTransformBone";
}

Unigine::XmlPtr
WorldTransformBone::saveXml(const Unigine::XmlPtr& xmlNode) const
{
    xmlNode->addChild(Unigine::Xml::create("bone"))->setData(m_boneName.data());
    return Node::saveXml(xmlNode);
}

bool
WorldTransformBone::loadXml(const Unigine::XmlPtr& xml)
{
    int boneIdx = xml->findChild("bone");
    if (boneIdx != -1)
    {
        m_boneName = xml->getChild(boneIdx)->getData();
    }
    return Node::loadXml(xml);
}

Unigine::NodePtr
WorldTransformBone::createUnigine() const
{
    auto result = Unigine::WorldTransformBone::create(m_name.data());
    result->setBoneName(m_boneName.data());
    applyUnigineNodeParams(result);
    return result;
}

// NodeNotSupported

Unigine::XmlPtr NodeNotSupported::saveXml(const Unigine::XmlPtr& xml) const
{
    if (m_xmlData)
    {
        return m_xmlData;
    }
    return xml;
}

bool NodeNotSupported::loadXml(const Unigine::XmlPtr& xml)
{
    m_xmlData = Xml::create();
    m_xmlData->copy(xml);
    for (int xmlIndex = 0; xmlIndex < m_xmlData->getNumChildren(); ++xmlIndex)
    {
        const auto& xmlChild = m_xmlData->getChild(xmlIndex);
        if (!strcmp(xmlChild->getName(), "node"))
        {
            m_xmlData->removeChild(xmlChild);
        }
    }
    return true;
}

Unigine::NodePtr NodeNotSupported::createUnigine() const
{
    throw(std::logic_error("Trying to createUnigine() of unsupported node type"));
    return {};
}

}//namespace MetaUnigine
