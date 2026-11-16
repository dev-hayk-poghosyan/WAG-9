#pragma once

#include "AnimationTriggerPlaybackType.pb.h"

#include "neokon-shared/PlusEnum.h"

/**
 * Defines the animation playback sequence for animation triggers.
 */
enum class AnimationTriggerPlaybackType : int
{
    /**
     * Plays the animation from first to last frame.
     */
    Forward = proto::AnimationTriggerPlaybackType::FORWARD,

    /**
     * Plays the animation from last to first frame.
     */
    Backward = proto::AnimationTriggerPlaybackType::BACKWARD
};

inline AnimationTriggerPlaybackType
animationTriggerPlaybackTypeFromProto(proto::AnimationTriggerPlaybackType::Type playbackType)
{
    static_assert(+AnimationTriggerPlaybackType::Forward ==
        proto::AnimationTriggerPlaybackType::FORWARD);
    static_assert(+AnimationTriggerPlaybackType::Backward ==
        proto::AnimationTriggerPlaybackType::BACKWARD);

    return static_cast<AnimationTriggerPlaybackType>(playbackType);
}

inline proto::AnimationTriggerPlaybackType::Type
protoFromAnimationTriggerPlaybackType(AnimationTriggerPlaybackType playbackType)
{
    static_assert(+AnimationTriggerPlaybackType::Forward ==
        proto::AnimationTriggerPlaybackType::FORWARD);
    static_assert(+AnimationTriggerPlaybackType::Backward ==
        proto::AnimationTriggerPlaybackType::BACKWARD);

    return static_cast<proto::AnimationTriggerPlaybackType::Type>(playbackType);
}

