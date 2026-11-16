#include "RailSwitchInfoComponents.h"

#include <string>

REGISTER_COMPONENT(RailSwitchPointRail);
REGISTER_COMPONENT(RailSwitchRod);

REGISTER_COMPONENT(RailSwitchPartTransforms);

using namespace Unigine;

namespace
{

Math::dmat4
componentVariableArrayDoubleToDMat4(ComponentVariableArray<ComponentVariableDouble>& values)
{
    Math::Mat4 matrix;

    if (values.size() == 16)
    {
        matrix.m00 = values[0];
        matrix.m01 = values[4];
        matrix.m02 = values[8];
        matrix.m03 = values[12];
        matrix.m10 = values[1];
        matrix.m11 = values[5];
        matrix.m12 = values[9];
        matrix.m13 = values[13];
        matrix.m20 = values[2];
        matrix.m21 = values[6];
        matrix.m22 = values[10];
        matrix.m23 = values[14];
    }

    return matrix;
}

void
componentVariableArrayDoubleFromDMat4(ComponentVariableArray<ComponentVariableDouble>& values,
    const Math::dmat4& matrix)
{
    values.resize(16);

    values[0] = matrix.m00;
    values[4] = matrix.m01;
    values[8] = matrix.m02;
    values[12] = matrix.m03;
    values[1] = matrix.m10;
    values[5] = matrix.m11;
    values[9] = matrix.m12;
    values[13] = matrix.m13;
    values[2] = matrix.m20;
    values[6] = matrix.m21;
    values[10] = matrix.m22;
    values[14] = matrix.m23;
}

}//Anonymous namespace

void
RailSwitchPartTransforms::setLeftTransform(const Math::Mat4& transform)
{
    componentVariableArrayDoubleFromDMat4(LeftTransform, transform);
}

void
RailSwitchPartTransforms::setRightTransform(const Math::Mat4& transform)
{
    componentVariableArrayDoubleFromDMat4(RightTransform, transform);
}

Math::Mat4
RailSwitchPartTransforms::getLeftTransform()
{
    return componentVariableArrayDoubleToDMat4(LeftTransform);
}

Math::Mat4
RailSwitchPartTransforms::getRightTransform()
{
    return componentVariableArrayDoubleToDMat4(RightTransform);
}

void
railSwitchToRight(Unigine::NodePtr srcNode, const char* suffix)
{
    ComponentSystem* componentSystem = ComponentSystem::get();

    NodeReferencePtr nodeRef = checked_ptr_cast<NodeReference>(srcNode);
    NodePtr rootNode = nodeRef ? nodeRef->getReference() : srcNode;
    int numChildren = rootNode->getNumChildren();

    for (int i = 0; i < numChildren; i++)
    {
        NodePtr childNode = rootNode->getChild(i);
        std::string name = childNode->getName();
        if (name.find(suffix) == std::string::npos)
        {
            continue;
        }

        auto transformsInfo = componentSystem->getComponent<RailSwitchPartTransforms>(childNode);
        if (transformsInfo)
        {
            childNode->setTransform(transformsInfo->getRightTransform());
            continue;
        }

        // Old code with adding transform to base
        Math::Mat4 transform = Math::Mat4_identity;

        auto arrowRailInfo = componentSystem->getComponent<RailSwitchPointRail>(childNode);
        auto rodInfo = componentSystem->getComponent<RailSwitchRod>(childNode);

        if (arrowRailInfo)
        {
            //arrow rail
            const float angle = -arrowRailInfo->TurnToLeftDeg;
            transform = Math::Mat4(Math::quat(0.0f, 0.0f, angle));
        }

        if (rodInfo)
        {
            //rod
            const float offset = rodInfo->YOffsetToLeftM;
            transform.setTranslate(Math::Vec3_forward * offset);
        }

        Math::Mat4 prevTransform = childNode->getTransform();
        Math::Mat4 newTransform = prevTransform * transform;
        childNode->setTransform(newTransform);
    }
}

void
railSwitchToLeft(Unigine::NodePtr srcNode, const char* suffix)
{
    ComponentSystem* componentSystem = ComponentSystem::get();

    NodeReferencePtr nodeRef = checked_ptr_cast<NodeReference>(srcNode);
    NodePtr rootNode = nodeRef ? nodeRef->getReference() : srcNode;
    int numChildren = rootNode->getNumChildren();

    for (int i = 0; i < numChildren; i++)
    {
        NodePtr childNode = rootNode->getChild(i);
        std::string name = childNode->getName();
        if (name.find(suffix) == std::string::npos)
        {
            continue;
        }

        auto transformsInfo = componentSystem->getComponent<RailSwitchPartTransforms>(childNode);
        if (transformsInfo)
        {
            childNode->setTransform(transformsInfo->getLeftTransform());
            continue;
        }

        // Old code with adding transform to base
        Math::Mat4 transform = Math::Mat4_identity;

        auto arrowRailInfo = componentSystem->getComponent<RailSwitchPointRail>(childNode);
        auto rodInfo = componentSystem->getComponent<RailSwitchRod>(childNode);

        if (arrowRailInfo)
        {
            //arrow rail
            const float angle = arrowRailInfo->TurnToLeftDeg;
            transform = Math::Mat4(Math::quat(0.0f, 0.0f, angle));
        }

        if (rodInfo)
        {
            //rod
            const float offset = -rodInfo->YOffsetToLeftM;
            transform.setTranslate(Math::Vec3_forward * offset);
        }

        Math::Mat4 prevTransform = childNode->getTransform();
        Math::Mat4 newTransform = prevTransform * transform;
        childNode->setTransform(newTransform);
    }
}
