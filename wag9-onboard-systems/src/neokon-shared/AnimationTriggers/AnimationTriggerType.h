#pragma once

#include "AnimationTriggerType.pb.h"

#include "neokon-shared/PlusEnum.h"

/**
 * Types of animation trigger.
 * For now supported only two trigger types: 0 - train, 1 - camera.
 * @see http://git.neokon.local:22580/root/GlobalTerrainEditor-release/wikis/world/anim-switch-triggers
 */
enum class AnimationTriggerType : int
{
    /**
     * For triggers on rail paths which must be passed by train.
     */ 
    Train = proto::AnimationTriggerType::TRAIN,

    /**
     * For triggers in world which must be intersected by player's capsule.
     */ 
    Camera = proto::AnimationTriggerType::CAMERA
};

inline AnimationTriggerType
animationTriggerTypeFromProto(proto::AnimationTriggerType::Type triggerType)
{
    static_assert(+AnimationTriggerType::Train == proto::AnimationTriggerType::TRAIN);
    static_assert(+AnimationTriggerType::Camera == proto::AnimationTriggerType::CAMERA);

    return static_cast<AnimationTriggerType>(triggerType);
}

inline proto::AnimationTriggerType::Type
protoFromAnimationTriggerType(AnimationTriggerType triggerType)
{
    static_assert(+AnimationTriggerType::Train == proto::AnimationTriggerType::TRAIN);
    static_assert(+AnimationTriggerType::Camera == proto::AnimationTriggerType::CAMERA);

    return static_cast<proto::AnimationTriggerType::Type>(triggerType);
}

