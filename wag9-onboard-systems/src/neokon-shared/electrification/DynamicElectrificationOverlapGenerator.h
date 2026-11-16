#pragma once

#include "DynamicElectrification.h"

#include "neokon-shared/electrification/ElectrificationSetEnumTypes.h"
#include "neokon-shared/electrification/ElectrificationGeneratorDataTypes.h"
#include "neokon-shared/electrification/DynamicElectrificationCatenaryGenerator.h"
#include <memory>

#include <UnigineNodes.h>
#include <UnigineMathLib.h>

/**
 * Генератор сопряжений анкерных участков.
 * Добавляет переходные опоры и анкеровку граничных опор сопрягаемых
 * анкерных участков.
 * Опоры могут быть сгенерированны в виде нод, добавляемых к корневой ноде, либо в виде
 * экземпляров кластеров. Помимо опор также генерирует меши подвесок, используя для этого
 * генератор подвесок DynamicElectrificationCatenaryGenerator.
 */
class DynamicElectrificationOverlapGenerator : public DynamicElectrificationPart
{
public:
    struct CreateArgs : electrification::PoleGenerationArgs
    {
        electrification::TensioningType startTensioningType{ electrification::TensioningType::Fixed };
        electrification::TensioningType endTensioningType{ electrification::TensioningType::Fixed };
        electrification::CatenaryType startCatenaryType{ electrification::CatenaryType::Simple };
        electrification::CatenaryType endCatenaryType{ electrification::CatenaryType::Simple };
    };

public:
    DynamicElectrificationOverlapGenerator(uint32_t id, CreateArgs&& args);

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

    std::optional<electrification::CatenaryContextData> getAnchorCatenaryData(
        electrification::DynamicPoleGenInfo const& anchorPoleGenInfo,
        electrification::DynamicPoleGenInfo const& transitionalPoleGenInfo,
        electrification::TensioningType tensioningType, Unigine::Math::Mat4 const tensioningMount);

    void maybeCreateRootNode();
    void createPole(electrification::DynamicPoleGenInfo const& info,
        CreateInstanceFunc const& sink) const;
    /**
     * We had to use the separate method to allow additional 180-degree rotation for clusters.
     * TODO: fix the need of such rotation.
     */
    void createAnchors(std::vector<electrification::DynamicPoleGenInfo> const& poleGenInfo,
        CreateInstanceFunc const& sink) const;

    const CreateArgs m_genParams;
    std::vector<electrification::DynamicPoleGenInfo> m_poleGenInfo;
    std::vector<DynamicElectrificationCatenaryGeneratorPtr> m_catenarySegments;
    std::optional<Unigine::Math::Mat4> m_startTensioningMount;
    std::optional<Unigine::Math::Mat4> m_endTensioningMount;
    std::optional<electrification::CatenaryContextData> m_startAnchorCatenaryData;
    std::optional<electrification::CatenaryContextData> m_endAnchorCatenaryData;

    Unigine::NodePtr m_rootNode;
};
