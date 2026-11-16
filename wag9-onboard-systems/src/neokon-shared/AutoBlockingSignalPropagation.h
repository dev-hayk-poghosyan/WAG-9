#pragma once

#include "AutoBlockingSignalPropagationMap.pb.h"

/**
 * Defines how auto-blocking signal is propagated through a particular area.
 */
enum class AutoBlockingSignalPropagation
{
    /**
     * The signal can't be receved and is not propagated further.
     *
     * @note We rely on AutoBlockingSignalPropagation() == AutoBlockingSignalPropagation::SignalLoss.
     */
    SignalLoss = proto::AutoBlockingSignalPropagationMap::SIGNAL_LOSS,

    /**
     * The signal can't be received, but is still propagated further.
     * This happens when the signal goes via a cable, rathar than via rails.
     */
    SignalBypass = proto::AutoBlockingSignalPropagationMap::SIGNAL_BYPASS,

    /**
     * The signal is propagated normally.
     */
    Normal = proto::AutoBlockingSignalPropagationMap::NORMAL
};

constexpr proto::AutoBlockingSignalPropagationMap::SignalPropagation
autoBlockingSignalPropagationToProto(AutoBlockingSignalPropagation propagation)
{
    return static_cast<proto::AutoBlockingSignalPropagationMap::SignalPropagation>(propagation);
}

constexpr AutoBlockingSignalPropagation
autoBlockingSignalPropagationFromProto(
    proto::AutoBlockingSignalPropagationMap::SignalPropagation propagation)
{
    return static_cast<AutoBlockingSignalPropagation>(propagation);
}
