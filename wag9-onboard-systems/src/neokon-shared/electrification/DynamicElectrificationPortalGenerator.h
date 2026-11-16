#pragma once

#include "DynamicElectrification.h"

#include "neokon-shared/electrification/DynamicElectrificationCatenaryGenerator.h"
#include "neokon-shared/electrification/ElectrificationContextDataTypes.h"
#include "neokon-shared/electrification/ElectrificationGeneratorDataTypes.h"

#include <memory>
#include <UnigineNodes.h>
#include <UnigineMathLib.h>

class DynamicElectrificationPortalGenerator;
using DynamicElectrificationPortalGeneratorPtr =
    std::shared_ptr<DynamicElectrificationPortalGenerator>;

/**
 * Класс генератора жестких поперечины (порталов).
 * Класс создает множество нод (или экземпляры кластеров) жестких поперечин
 * в пределах заданной области. Например -  станции. Для того, чтобы класс-хост
 * мог обновить генератор, он сравнивает его контрольное число - uniqueGenValue,
 * с таковым у предыдущей версии генератора. За обновление этой метки отвечает код,
 * создающий и инициализирующий генератор на фронтэнде. 
 */
class DynamicElectrificationPortalGenerator : public DynamicElectrificationPart
{
public:
    struct CreateArgs : electrification::PortalGenerationArgs
    {
        int uniqueGenValue{0};
    };

public:
    DynamicElectrificationPortalGenerator(uint32_t id, CreateArgs&& args);

    virtual bool load() override;
    virtual void unload() override;
    virtual Unigine::NodePtr createNodes() override;
    virtual bool isDifferentWith(const DynamicRailwayPartPtr& old) const override;
    virtual void validateClusters(GetOrCreateClusterFunc const& getOrCreateCluster,
        std::optional<Unigine::Math::Vec3> const& center = std::nullopt) const override;

    int getUniqueGenValue() const { return m_genParams.uniqueGenValue; }

private:
    using CreateInstanceFunc = std::function<void(std::string const& nodePath,
        Unigine::Math::Mat4 const& worldTransform)>;

    struct NodePathWithTransform
    {
        std::string path;
        Unigine::Math::Mat4 transform;
    };

    struct CreatePortalData
    {
        NodePathWithTransform const rigel;
        NodePathWithTransform const leftPole;
        NodePathWithTransform const rightPole;
        std::optional<DynamicElectrificationCatenaryGenerator::CreateArgs> spanWireArgs;

        void listNodes(CreateInstanceFunc const& sink) const
        {
            sink(rigel.path, rigel.transform);
            sink(leftPole.path, leftPole.transform);
            sink(rightPole.path, rightPole.transform);
        }
    };

    struct CreateCrossSpanData
    {
        NodePathWithTransform const leftPole;
        NodePathWithTransform const rightPole;
        std::optional<DynamicElectrificationCatenaryGenerator::CreateArgs> catenaryArgs;
        std::optional<DynamicElectrificationCatenaryGenerator::CreateArgs> spanWireArgs;

        void listNodes(CreateInstanceFunc const& sink) const
        {
            sink(leftPole.path, leftPole.transform);
            sink(rightPole.path, rightPole.transform);
        }
    };

    void maybeCreateRootNode();

    /**
     * @returns std::nullopt if there was any problem.
     */
    std::optional<CreatePortalData> preparePortal(
        electrification::DynamicPortalGenInfo const& info) const;

    /**
     * @returns std::nullopt if there was any problem.
     */
    std::optional<CreateCrossSpanData> prepareCrossSpan(
        electrification::DynamicPortalGenInfo const& info) const;

    const CreateArgs m_genParams;

    std::vector<CreatePortalData> m_portalCreatesForClusters;
    std::vector<CreateCrossSpanData> m_crossSpanCreatesForClusters;
    std::vector<DynamicElectrificationCatenaryGeneratorPtr> m_wireGenerators;

    Unigine::NodePtr m_rootNode;
};
