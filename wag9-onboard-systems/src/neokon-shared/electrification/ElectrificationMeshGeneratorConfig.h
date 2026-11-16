#pragma once

#include "ElectrificationPresetEnumTypes.h"

#include <memory>
#include <optional>
#include <string>

/**
 * Данный класс загружает пресеты - конфигурационные файлы для динамической генерации контента,
 * элементов контактной сети железных дорог (электрификации).Загружаемый формат пресетов описан
 * в следующих скриптах:
 *
 * -neokon-shared/proto/ElectrificationPreset.proto
 * -neokon-shared/proto/ElectrificationPresetAnchor
 * -neokon-shared/proto/ElectrificationPresetCatenary
 * -neokon-shared/proto/ElectrificationPresetCommonParams
 * -neokon-shared/proto/ElectrificationPresetInsulator
 * -neokon-shared/proto/ElectrificationPresetPole
 * -neokon-shared/proto/ElectrificationPresetPortal
 * -neokon-shared/proto/ElectrificationPresetPortalSuspension
 * -neokon-shared/proto/ElectrificationPresetSpanwire
 */
class ElectrificationMeshGeneratorConfig
{
public:
    /**
     * Общие параметры, затрагивающие все элементы контактной сети.
     */
    struct CommonParamsData
    {
    public:
        void loadFromProto(proto::electrification::preset::CommonParams const& dataBlock);
        void saveToProto(proto::electrification::preset::CommonParams& dataBlock) const;

        electrification::ProjectCode projectCode;//код проекта.
        electrification::ContactWireType catenaryWireType;//индекс типа подвески.
        electrification::ConsoleProfileCut consoleProfileCut;//профиль балок для консолей.
    };

    /**
     * Блок данных для изоляторов всех типов и видов.
     */
    struct InsulatorData
    {
    public:
        void loadFromProto(proto::electrification::preset::Insulator const& dataBlock);
        void saveToProto(proto::electrification::preset::Insulator& dataBlock) const;

        electrification::InsulatorType type;//тип изолятора
        electrification::InsulatorConstruct construct;//конструкция изолятора
        electrification::InsulatorMaterial material;//материал изолятора
        int allowedVoltage{ 0 };//предельное допустимое напряжение в киловольтах.
    };

    /**
     * Блок данных для фиксаторного троса под жесткой поперечиной.
     */
    struct SpanwireData
    {
    public:
        void loadFromProto(proto::electrification::preset::Spanwire const& dataBlock);
        void saveToProto(proto::electrification::preset::Spanwire& dataBlock) const;

        //расположение изолятора на тросе относительно фиксатора контактного провода.
        electrification::SpanwireInsulatorPlacement insulatorPlacement;
        InsulatorData spanwireInsulator;//изолятора для троса.
    };

    /**
     * Блок данных для подвесок проводов.
     * При генерации подвески её тип (рессорный/безрессорный)
     * берется из файла ElectrificationSet.json, а количество проводов
     * - из блока данных CommonParams.
     */
    struct CatenaryData
    {
    public:
        void loadFromProto(proto::electrification::preset::Catenary const& dataBlock);
        void saveToProto(proto::electrification::preset::Catenary& dataBlock) const;

        //натяжной изолятор для контактного провода и несущего троса.
        std::optional<InsulatorData> tensionInsulator;
        //секционный изолятор для воздушных стрелок.
        std::optional<InsulatorData> sectionInsulator;

        double wireDiameter{0.0};
        double cableDiameter{0.0};
        double dropperDiameter{0.0};
        
    };

    /**
     * Блок данных для опоры.
     */
    struct PoleData
    {
    public:
        void loadFromProto(proto::electrification::preset::Pole const& dataBlock);
        void saveToProto(proto::electrification::preset::Pole& dataBlock) const;

        InsulatorData cantileverInsulator;//изолятор для консоли.
        electrification::PillarMaterial pillarMaterial;//конструкция опоры.
        std::string nodePrefixHeight;//строковый преффикс высоты опоры.
        std::string nodePrefixFoundationColor;//строковый преффикс окраски основания.
    };

    /**
     * Блок данных для жесткой поперечины.
     */
    struct PortalData
    {
    public:
        void loadFromProto(proto::electrification::preset::Portal const& dataBlock);
        void saveToProto(proto::electrification::preset::Portal& dataBlock) const;

        electrification::PortalBlockType blockType;//тип блока фермы.
        electrification::PortalBlockMaterial blockMaterial;//материал блока.
        electrification::PillarMaterial pillarMaterial;//материал боковых опор-стоек.
        electrification::PillarType leftPillarType;//тип левой стойки.
        electrification::PillarType rightPillarType;//тип правой стойки.
        std::optional<SpanwireData> spanwire;//несущий трос (может отсутствовать).
        std::string nodePrefixHeight;//строковый префикс высоты опоры.
        std::string nodePrefixFoundationColor;//строковый префикс окраски основания.
    };

    /**
     * Блок данных для подвеса под жесткую поперечину -
     * консольные и фиксаторные стойки, а также фиксаторы на тросе. 
     */
    struct PortalSuspensionData
    {
    public:
        void loadFromProto(proto::electrification::preset::PortalSuspension const& dataBlock);
        void saveToProto(proto::electrification::preset::PortalSuspension& dataBlock) const;

        std::string nodeName;//уникальное имя ноды подвеса.
    };

    /**
     * Блок данных для анкеровки (компенсирующего устройства на опоре).
     */
    struct AnchorData
    {
    public:
        void loadFromProto(proto::electrification::preset::Anchor const& dataBlock);
        void saveToProto(proto::electrification::preset::Anchor& dataBlock) const;

        /**
         * Изолятор для анкерного блока.
         * Поскольку изолятор является частью модели блока,
         * параметры изолятора используются для подбора модели блока,
         * а не самого изолятора.
         */
        InsulatorData tensionInsulator;

        std::string backTieNode;//имя ноды для анкерной оттяжки.
        std::string fixedAnchorBlockNodeName;//имя ноды блока для жесткой анкеровки.
        std::string compensatedAnchorBlockNodeName;//имя ноды блока для компенсированной а.
        std::string semiCompensatedAnchorBlockNodeName;//имя ноды блока для полу-компенс. анкеровки.
    };

public:
    bool fillPropertiesFromJsonFile(const std::string& meshGenConfigFile);
    bool savePropertiesToJsonFile(const std::string& meshGenConfigFile);

    const CommonParamsData& getCommonParamsData() const { return m_commonParamsData; }
    const CatenaryData& getCatenaryData() const { return m_catenaryData; }
    const PoleData& getPoleData() const { return m_poleData; }
    const PortalData& getPortalData() const { return m_portalData; }
    const PortalSuspensionData& getPortalSuspensionData() const { return m_portalSuspensionData; }
    const AnchorData& getAnchorData() const { return m_anchorData; }

private:
    CommonParamsData m_commonParamsData;
    CatenaryData m_catenaryData;
    PoleData m_poleData;
    PortalData m_portalData;
    PortalSuspensionData m_portalSuspensionData;
    AnchorData m_anchorData;
};
