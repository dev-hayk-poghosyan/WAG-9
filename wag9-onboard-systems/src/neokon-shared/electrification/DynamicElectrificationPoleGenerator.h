#pragma once

#include "DynamicElectrification.h"

#include "neokon-shared/electrification/ElectrificationGeneratorDataTypes.h"
#include "neokon-shared/electrification/DynamicElectrificationCatenaryGenerator.h"
#include <memory>

#include <UnigineNodes.h>
#include <UnigineMathLib.h>

/**
 * Класс-генератор анкерных участков на перегонах. Генерирует
 * опоры в пределах одного анкерного участка, а также среднюю анкеровку.
 * НЕ генерирует анкеровку на граничных порах -  за ней отвечает генератор
 * сопряжений DynamicElectrificationOverlapGenerator.
 * Опоры могут быть сгенерированны в виде нод, добавляемых к корневой ноде, либо в виде
 * экземпляров кластеров. Помимо опор также генерирует меши подвесок, используя для этого
 * генератор подвесок DynamicElectrificationCatenaryGenerator.
 */
class DynamicElectrificationPoleGenerator : public DynamicElectrificationPart
{
public:
    struct CreateArgs : electrification::PoleGenerationArgs
    {
        electrification::TensioningType middleTensioning{electrification::TensioningType::Compensated};
    };

public:
    DynamicElectrificationPoleGenerator(uint32_t id, CreateArgs&& args);

    virtual bool load() override;
    virtual void unload() override;
    virtual Unigine::NodePtr createNodes() override;
    virtual bool isDifferentWith(const DynamicRailwayPartPtr& old) const override;
    virtual void validateClusters(GetOrCreateClusterFunc const& getOrCreateCluster,
        std::optional<Unigine::Math::Vec3> const& center = std::nullopt) const override;

private:
    using CreateInstanceFunc = std::function<void(std::string const& nodePath,
        Unigine::Math::Mat4 const& worldTransform)>;

    void loadCatenarySegments();
    void loadTensioningMounts();
    void loadAnchorBlocksCatenaryData();

    void maybeCreateRootNode();
    void createPoles(std::vector<electrification::DynamicPoleGenInfo> const& poleGenInfo,
        CreateInstanceFunc const& sink) const;

    /**
     * We had to use the separate method to allow additional 180-degree rotation for clusters.
     * TODO: fix the need of such rotation.
     */
    void createAnchors(
        std::vector<electrification::DynamicPoleGenInfo> const& poleGenInfo,
        CreateInstanceFunc const& sink) const;

    const CreateArgs m_genParams;
    std::vector<DynamicElectrificationCatenaryGeneratorPtr> m_catenarySegments;
    std::optional<Unigine::Math::Mat4> m_startTensioningMount;
    std::optional<Unigine::Math::Mat4> m_endTensioningMount;
    std::optional<electrification::CatenaryContextData> m_startAnchorCatenaryData;
    std::optional<electrification::CatenaryContextData> m_endAnchorCatenaryData;
    int m_startAnchorPoleIdx{-1};
    int m_endAnchorPoleIdx{-1};
    int m_middleAnchorPoleIdx{-1};

    Unigine::NodePtr m_rootNode;
};
