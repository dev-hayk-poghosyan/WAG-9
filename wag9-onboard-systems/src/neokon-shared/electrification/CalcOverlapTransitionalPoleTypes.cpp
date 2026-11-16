#include "CalcOverlapTransitionalPoleTypes.h"

void
calcOverlapTransitionalPoleTypes(electrification::TensionOverlapType overlapType,
    electrification::PoleSide poleSide,
    std::vector<electrification::DynamicPoleGenInfo>& poleGenInfo)
{
    assert(poleGenInfo.size() >= 4);

    const bool isInsulated = overlapType == electrification::TensionOverlapType::Insulated;

    int firstPoleIdx = 1;
    int lastPoleIdx = poleGenInfo.size() - 2;
    for (int i = firstPoleIdx; i <= lastPoleIdx; i++)
    {
        poleGenInfo[i].poleType = electrification::PoleType::Transitional;
        if (i == firstPoleIdx)
        {
            if (poleSide == electrification::PoleSide::RightHand)
            {
                poleGenInfo[i].transitionalPoleSubtype = isInsulated
                    ? electrification::TransitionalPoleType::TransitionalAInsulated
                    : electrification::TransitionalPoleType::TransitionalA;
            }
            else
            {
                poleGenInfo[i].transitionalPoleSubtype = isInsulated
                    ? electrification::TransitionalPoleType::TransitionalBInsulated
                    : electrification::TransitionalPoleType::TransitionalB;
            }
        }
        else if (i == lastPoleIdx)
        {
            if (poleSide == electrification::PoleSide::RightHand)
            {
                poleGenInfo[i].transitionalPoleSubtype = isInsulated
                    ? electrification::TransitionalPoleType::TransitionalBInsulated
                    : electrification::TransitionalPoleType::TransitionalB;
            }
            else
            {
                poleGenInfo[i].transitionalPoleSubtype = isInsulated
                    ? electrification::TransitionalPoleType::TransitionalAInsulated
                    : electrification::TransitionalPoleType::TransitionalA;
            }
        }
        else
        {
            poleGenInfo[i].transitionalPoleSubtype = isInsulated
                ? electrification::TransitionalPoleType::TransitionalCInsulated
                : electrification::TransitionalPoleType::TransitionalC;
        }
    }
}
