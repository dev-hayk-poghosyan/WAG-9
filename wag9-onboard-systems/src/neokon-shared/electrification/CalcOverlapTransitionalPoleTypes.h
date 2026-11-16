#pragma once

#include "neokon-shared/electrification/ElectrificationGeneratorDataTypes.h"
#include "neokon-shared/electrification/ElectrificationSetEnumTypes.h"
#include <vector>

void calcOverlapTransitionalPoleTypes(electrification::TensionOverlapType overlapType,
    electrification::PoleSide poleSide,
    std::vector<electrification::DynamicPoleGenInfo>& poleGenInfo);
