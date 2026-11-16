#pragma once

/**
 * Copy useful parameters from one object's surface to other's
 * Can be used to create ObjectMeshCluster from template ObjectMeshStatic, etc.
 * Some rarely used parameters are not copied
 * 
 * @param surfaceIndex - index of surface to work with.
 * @param visibleDistance - maximum possible visible distance from observer.
 * ScrType & DstType can be any, inherited from Unigine:: or MetaUnigine:: Object
 */
template<class SrcType, class DstType>
void copySurfaceParametersFromTo(SrcType sourceObject,
                                 DstType destinationObject,
                                 int surfaceIndex, 
                                 float visibleDistance = std::numeric_limits<float>::max())
{
    float meshMaxVisibleDistance = sourceObject->getMaxVisibleDistance(surfaceIndex);
    if (meshMaxVisibleDistance > visibleDistance)
    {
        meshMaxVisibleDistance = visibleDistance;
    }

    destinationObject->setMaterial(sourceObject->getMaterial(surfaceIndex), surfaceIndex);

    destinationObject->setEnabled(sourceObject->isEnabled(surfaceIndex), surfaceIndex);

    destinationObject->setMinParent(sourceObject->getMinParent(surfaceIndex),
        surfaceIndex);
    destinationObject->setMaxParent(sourceObject->getMaxParent(surfaceIndex),
        surfaceIndex);

    destinationObject->setCastShadow(sourceObject->getCastShadow(surfaceIndex),
        surfaceIndex);

    destinationObject->setCastWorldShadow(
        sourceObject->getCastWorldShadow(surfaceIndex), surfaceIndex);
    destinationObject->setCastEnvProbeShadow(
        sourceObject->getCastEnvProbeShadow(surfaceIndex), surfaceIndex);

    destinationObject->setBakeToGI(sourceObject->getBakeToGI(surfaceIndex),
        surfaceIndex);
    destinationObject->setBakeToEnvProbe(
        sourceObject->getBakeToEnvProbe(surfaceIndex), surfaceIndex);

    destinationObject->setShadowMask(sourceObject->getShadowMask(surfaceIndex),
        surfaceIndex);
    destinationObject->setShadowMode(sourceObject->getShadowMode(surfaceIndex),
        surfaceIndex);

    destinationObject->setIntersection(sourceObject->getIntersection(surfaceIndex),
        surfaceIndex);
    destinationObject->setIntersectionMask(sourceObject->getIntersectionMask(surfaceIndex),
        surfaceIndex);

    destinationObject->setCollision(sourceObject->getCollision(surfaceIndex), surfaceIndex);
    destinationObject->setCollisionMask(sourceObject->getCollisionMask(surfaceIndex), surfaceIndex);

    destinationObject->setPhysicsIntersection(
        sourceObject->getPhysicsIntersection(surfaceIndex), surfaceIndex);
    destinationObject->setPhysicsIntersectionMask(
        sourceObject->getPhysicsIntersectionMask(surfaceIndex), surfaceIndex);

    destinationObject->setViewportMask(sourceObject->getViewportMask(surfaceIndex), surfaceIndex);

    destinationObject->setMinVisibleDistance(sourceObject->getMinVisibleDistance(surfaceIndex),
        surfaceIndex);
    destinationObject->setMaxVisibleDistance(meshMaxVisibleDistance, surfaceIndex);

    destinationObject->setMinFadeDistance(sourceObject->getMinFadeDistance(surfaceIndex),
        surfaceIndex);
    destinationObject->setMaxFadeDistance(sourceObject->getMaxFadeDistance(surfaceIndex),
        surfaceIndex);
}
