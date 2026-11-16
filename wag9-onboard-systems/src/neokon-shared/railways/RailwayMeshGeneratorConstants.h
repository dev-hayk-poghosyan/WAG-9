#pragma once

#include <UnigineMathLib.h>
#include <string>
#include <vector>
#include <optional>

const int JUNCTION_TYPE_Y = 0;
const int JUNCTION_TYPE_X = 1;
const int JUNCTION_TYPE_CROSS = 2;
const int JUNCTION_TYPE_THREEWAY = 3;

std::vector<Unigine::Math::Vec3> const& railwayEmbankmentFacePositions();

std::vector<Unigine::Math::vec2> const& railwayEmbankmentFaceUVCoords();

std::vector<Unigine::Math::Vec3> const& railFacePositionsLOD0();

std::vector<Unigine::Math::vec2> const& railFaceUVCoordsLOD0();

std::vector<Unigine::Math::Vec3> const& railFacePositionsLOD1();

std::vector<Unigine::Math::vec2> const& railFaceUVCoordsLOD1();

std::vector<int> const& pointRailStandartLengths();

std::optional<int> pointRailSlicePoint(int lengthMM);

std::optional<int> pointRailNumSleepers(int lengthMM);

std::optional<int> pointRailSleeperInterval(int lengthMM);

int chooseSuitablePointRailLength(int avaibleSpaceMM);

enum class DynamicRailwayPartType
{
    BranchEmbankment,
    BranchRails,
    BranchAdditional,
    BranchSleepers,
    BranchFishPlates,
    JunctionEmbankment,
    JunctionRails,
    JunctionAdditional,
    JunctionClusters,
    Total
};

bool isDynamicRailwayPartTypeEnabled(DynamicRailwayPartType type);
void setDynamicRailwayPartTypeEnabled(DynamicRailwayPartType type, bool value);
