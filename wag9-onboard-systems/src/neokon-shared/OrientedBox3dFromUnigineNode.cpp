#include "OrientedBox3dFromUnigineNode.h"

#include "GlmUnigineInterop.h"

#include <glm/glm.hpp>

#include <UnigineBounds.h>
#include <UnigineMathLib.h>

#include <cassert>

OrientedBox3D orientedBox3dFromUnigineNode(Unigine::NodePtr const& node)
{
    assert(node);

    using namespace Unigine;

    BoundBox const boundBox = node->getBoundBox();
    Math::Vec3 const halfExtents((boundBox.getMax() - boundBox.getMin()) * 0.5);
    Math::Vec3 const centerInLocalCoords = Math::Vec3(boundBox.getMin()) + halfExtents;

    Math::Mat4 const worldTransform = node->getWorldTransform();
    Math::Vec3 const centerInWorldCoords = worldTransform * centerInLocalCoords;

    glm::dmat3 const axes(
        toGlm(worldTransform.getAxisX()),
        toGlm(worldTransform.getAxisY()),
        toGlm(worldTransform.getAxisZ()));

    return OrientedBox3D(toGlm(centerInWorldCoords), axes, toGlm(halfExtents));
}
