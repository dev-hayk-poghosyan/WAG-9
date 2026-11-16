#include "UpdateDistanceBasedMaterials.h"

#include <boost/functional/hash.hpp>

using namespace Unigine;

namespace
{
std::string const baseMatName("sleepers_distance_tex_switch");
std::string const distanceParamName("replacement_dist");

struct MaterialSwitchDistance
{
    MaterialPtr material;
    float replacementDist{0.0f};
};

HashMap<UGUID, MaterialSwitchDistance> materialDistancesByUguid;
} // anonymous namespace

void
maybeAddDistanceBasedMaterial(Unigine::MaterialPtr const& materialCandidate)
{
    MaterialPtr baseMat = materialCandidate->getBaseMaterial();
    if (baseMatName != baseMat->getName())
    {
        // Not our material.
        return;
    }

    int const paramIdx = materialCandidate->findParameter(distanceParamName.c_str());
    if (paramIdx < 0)
    {
        // Invalid material with no required parameter.
        // Shouldn't happen for our basemat, but possible if parameter has been renamed.
        return;
    }

    float relacementDist{0.0f};

    UGUID const uguid = materialCandidate->getGUID();
    auto materialIter = materialDistancesByUguid.find(uguid);
    if (materialIter != materialDistancesByUguid.end())
    {
        // Test whether an existing material is still valid.
        // If not, replace with the @p materialCandidate.
        if (materialIter->data.material.isValid())
        {
            // Early-return if the material was already added.
            return;
        }
        else
        {
            materialIter->data.material = materialCandidate;
            materialIter->data.replacementDist = materialCandidate->getParameterFloat(paramIdx);

            relacementDist = materialIter->data.replacementDist;
        }
    }
    else
    {
        relacementDist = materialCandidate->getParameterFloat(paramIdx);

        materialDistancesByUguid.insert(uguid, {materialCandidate, relacementDist});
    }

    float const lodDistanceScale = Unigine::Render::getDistanceScale();
    float const distance = relacementDist * lodDistanceScale;

    materialCandidate->setParameterFloat(distanceParamName.c_str(), distance);
}

void
updateDistanceBasedMaterials()
{
    float const lodDistanceScale = Unigine::Render::getDistanceScale();

    Vector<UGUID> materialUguidsToRemove;

    for (auto& [hash, uguid, materialSwitchDistance] : materialDistancesByUguid)
    {
        float const distance = materialSwitchDistance.replacementDist * lodDistanceScale;

        if (materialSwitchDistance.material->isValid())
        {
            materialSwitchDistance.material->setParameterFloat(distanceParamName.c_str(),
                distance);
        }
        else
        {
            materialUguidsToRemove.append(uguid);
        }
    }

    // Remove invalid materials which could be appeared after reloading worlds.
    for (auto const& uguid : materialUguidsToRemove)
    {
        materialDistancesByUguid.erase(uguid);
    }
}
