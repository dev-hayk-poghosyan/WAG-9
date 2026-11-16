#pragma once

#include "DynamicElectrification.h"

#include "neokon-shared/electrification/ElectrificationGeneratorDataTypes.h"
#include "neokon-shared/electrification/DynamicElectrificationCatenaryGenerator.h"
#include <memory>

#include <UnigineNodes.h>
#include <UnigineMathLib.h>

class DynamicElectrificationManualWiresGenerator;
using DynamicElectrificationManualWiresGeneratorPtr =
    std::shared_ptr<DynamicElectrificationManualWiresGenerator>;

/**
 * Класс-генератор пользовательских участков подвески.
 * Класс создаёт множество нод (или экземпляры кластеров) для участков подвески в пределах заданной
 * области. Для того, чтобы класс-хост мог обновить генератор, он сравнивает
 * его контрольное число - uniqueGenValue, с таковым у предыдущей версии генератора.
 * За обновление этой метки отвечает код, создающий и инициализирующий генератор на фронтэнде.
 */
class DynamicElectrificationManualWiresGenerator : public DynamicElectrificationPart
{
public:
    struct CreateArgs : electrification::ManualWireGenerationArgs
    {
        int uniqueGenValue{0};
    };

public:
    DynamicElectrificationManualWiresGenerator(uint32_t id, CreateArgs&& args);

    virtual bool load() override;
    virtual void unload() override;
    virtual Unigine::NodePtr createNodes() override;
    virtual bool isDifferentWith(const DynamicRailwayPartPtr& old) const override;
    virtual void validateClusters(GetOrCreateClusterFunc const& getOrCreateCluster,
        std::optional<Unigine::Math::Vec3> const& center = std::nullopt) const override;

    int getUniqueGenValue() const { return m_genParams.uniqueGenValue; }

private:
    void maybeCreateRootNode();

    const CreateArgs m_genParams;

    std::vector<DynamicElectrificationCatenaryGeneratorPtr> m_catenarySegments;

    Unigine::NodePtr m_rootNode;
};
