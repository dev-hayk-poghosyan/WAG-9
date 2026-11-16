#pragma once

#include "SignallerExteriorCategory.pb.h"

#include "neokon-shared/PlusEnum.h"

/**
 * For now signallers has two exterior category.
 * These categories affect the signallers uniform, and therefore, their visual model.
 * @see proto-file message SignallerExteriorCategory.
 */
enum class SignallerExteriorCategory
{
    Subordinate = 0,
    Superior = 1
};

static_assert(
    +SignallerExteriorCategory::Subordinate == proto::SignallerExteriorCategory::Subordinate &&
    +SignallerExteriorCategory::Superior == proto::SignallerExteriorCategory::Superior,
    "SignallerExteriorCategory doesn't match proto::SignallerExteriorCategory::Category");

inline SignallerExteriorCategory
exteriorCategoryFromProto(proto::SignallerExteriorCategory::Category category)
{
    return static_cast<SignallerExteriorCategory>(category);
}

inline proto::SignallerExteriorCategory::Category
protoFromExteriorCategory(SignallerExteriorCategory category)
{
    return static_cast<proto::SignallerExteriorCategory::Category>(category);
}
