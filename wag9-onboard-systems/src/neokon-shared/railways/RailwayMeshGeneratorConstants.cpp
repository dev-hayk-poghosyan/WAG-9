#include "RailwayMeshGeneratorConstants.h"

#include "neokon-shared/PlusEnum.h"

#include <atomic>
#include <unordered_map>

using namespace Unigine;

const std::vector<Math::Vec3> k_railwayEmbankmentFacePositions
{
    Math::Vec3{ 0.0, 2.2, -0.6 },
    Math::Vec3{ 0.0, 1.6, -0.26 },
    Math::Vec3{ 0.0, 0.8, -0.22 },
    Math::Vec3{ 0.0, 0.0, -0.23 },
    Math::Vec3{ 0.0, -0.8, -0.22 },
    Math::Vec3{ 0.0, -1.6, -0.26 },
    Math::Vec3{ 0.0, -2.2, -0.6 }
};

const std::vector<Math::vec2> k_railwayEmbankmentFaceUVCoords
{
    Math::vec2{ 0.0f, 1.0f },
    Math::vec2{ 0.0f, 0.8529f },
    Math::vec2{ 0.0f, 0.6763f },
    Math::vec2{ 0.0f, 0.5f },
    Math::vec2{ 0.0f, 0.3237f },
    Math::vec2{ 0.0f, 0.1471f },
    Math::vec2{ 0.0f, 0.0f }
};

const std::vector<Math::Vec3>  k_railFacePositionsLOD0
{
    Math::Vec3{ 0.0, 0.075, -0.18 },
    Math::Vec3{ 0.0, 0.075, -0.169 },
    Math::Vec3{ 0.0, 0.012, -0.14944 },
    Math::Vec3{ 0.0, 0.012, -0.04312 },
    Math::Vec3{ 0.0, 0.036273, -0.034586 },
    Math::Vec3{ 0.0, 0.032, 0.0 },
    Math::Vec3{ 0.0, -0.032, 0.0 },
    Math::Vec3{ 0.0, -0.036273, -0.034586 },
    Math::Vec3{ 0.0, -0.012, -0.04312 },
    Math::Vec3{ 0.0, -0.012, -0.14944 },
    Math::Vec3{ 0.0, -0.075, -0.169 },
    Math::Vec3{ 0.0, -0.075, -0.18 },
};

const std::vector<Math::vec2>  k_railFaceUVCoordsLOD0
{
    Math::vec2{ 0.0f, 0.12756f },
    Math::vec2{ 0.0f, 0.12442f },
    Math::vec2{ 0.0f, 0.11026f },
    Math::vec2{ 0.0f, 0.08811f },
    Math::vec2{ 0.0f, 0.08209f },
    Math::vec2{ 0.0f, 0.07414f },
    Math::vec2{ 0.0f, 0.06032f },
    Math::vec2{ 0.0f, 0.05234f },
    Math::vec2{ 0.0f, 0.04611f },
    Math::vec2{ 0.0f, 0.02341f },
    Math::vec2{ 0.0f, 0.00892f },
    Math::vec2{ 0.0f, 0.00631f }
};

const std::vector<Math::Vec3>  k_railFacePositionsLOD1
{
    Math::Vec3{ 0.0, 0.075, -0.18 },
    Math::Vec3{ 0.0, 0.032, 0.0 },
    Math::Vec3{ 0.0, -0.032, 0.0 },
    Math::Vec3{ 0.0, -0.075, -0.18 }
};

const std::vector<Math::vec2>  k_railFaceUVCoordsLOD1
{
    Math::vec2{ 0.0f, 0.12756f },
    Math::vec2{ 0.0f, 0.07414f },
    Math::vec2{ 0.0f, 0.06032f },
    Math::vec2{ 0.0f, 0.0632f }
};

/**
 * Типовые длины остряков в миллимтрерах, упорядоченные
 * от самого длинного остряка, к самому короткому.
 */
const std::vector<int> k_arrowRailStandartLengthsMm
{
    15200, 8300, 5400
};

/**
 * Таблица смещений точек среза для остряков.
 * Точка среза - место, где профиль рельса на остряке
 * начинает сужаться к концу. Смещение точки среза
 * считается от корня остряка, в миллиметрах.
 * Ключ отображения - длина остряка.
 * Значение - смещение точки среза.
 */
const std::unordered_map<int, int> k_arrowRailsSlicePointsMm
{
    { 15200, 10300 },
    { 8300, 4200 },
    { 5400, 2200 },
    { 5200, 2200 }
};

const std::unordered_map<int, int> k_pointRailNumSleepers
{
    { 15200, 30 },
    { 8300, 16 },
    { 5400, 10 },
    { 5200, 10 }
};

const std::unordered_map<int, int> k_pointRailInterval
{
    { 15200, 494 },
    { 8300, 497 },
    { 5400, 500 },
    { 5200, 500 }
};

std::vector<Math::Vec3> const& railwayEmbankmentFacePositions()
{
    return k_railwayEmbankmentFacePositions;
}

std::vector<Math::vec2> const& railwayEmbankmentFaceUVCoords()
{
    return k_railwayEmbankmentFaceUVCoords;
}

std::vector<Math::Vec3> const& railFacePositionsLOD0()
{
    return k_railFacePositionsLOD0;
}

std::vector<Math::vec2> const& railFaceUVCoordsLOD0()
{
    return k_railFaceUVCoordsLOD0;
}

std::vector<Math::Vec3> const& railFacePositionsLOD1()
{
    return k_railFacePositionsLOD1;
}

std::vector<Math::vec2> const& railFaceUVCoordsLOD1()
{
    return k_railFaceUVCoordsLOD1;
}

std::vector<int> const& pointRailStandartLengths()
{
    return k_arrowRailStandartLengthsMm;
}

std::optional<int> pointRailSlicePoint(int lengthMM)
{
    auto foundIt = k_arrowRailsSlicePointsMm.find(lengthMM);
    if (foundIt != k_arrowRailsSlicePointsMm.end())
    {
        return foundIt->second;
    }

    return std::nullopt;
}

std::optional<int> pointRailNumSleepers(int lengthMM)
{
    auto foundIt = k_pointRailNumSleepers.find(lengthMM);
    if (foundIt != k_pointRailNumSleepers.end())
    {
        return foundIt->second;
    }

    return std::nullopt;
}

std::optional<int> pointRailSleeperInterval(int lengthMM)
{
    auto foundIt = k_pointRailInterval.find(lengthMM);
    if (foundIt != k_pointRailInterval.end())
    {
        return foundIt->second;
    }

    return std::nullopt;
}

int chooseSuitablePointRailLength(int avaibleSpaceMM)
{
    auto const& arrowRailsLengths = pointRailStandartLengths();
    int lastIdx = arrowRailsLengths.size() - 1;
    int result = arrowRailsLengths[lastIdx];

    for (size_t i = 0, numItems = arrowRailsLengths.size(); i < numItems; i++)
    {
        int lengthMM = pointRailStandartLengths()[i];
        if (lengthMM > avaibleSpaceMM)
        {
            continue;
        }

        result = lengthMM;
        break;
    }

    return result;
}

std::atomic_bool dynamicRailwaysPartTypeDisabled[+DynamicRailwayPartType::Total];

bool
isDynamicRailwayPartTypeEnabled(DynamicRailwayPartType type)
{
    if (+type >= +DynamicRailwayPartType::Total)
    {
        return true;
    }
    return !dynamicRailwaysPartTypeDisabled[+type].load(std::memory_order_relaxed);
}

void
setDynamicRailwayPartTypeEnabled(DynamicRailwayPartType type, bool value)
{
    if (+type >= +DynamicRailwayPartType::Total)
    {
        return;
    }

    dynamicRailwaysPartTypeDisabled[+type].store(!value, std::memory_order_relaxed);
}
