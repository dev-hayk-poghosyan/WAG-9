#pragma once

#include "ElectrificationContextDataTypes.h"
#include "ElectrificationAltMeshGeneratorConfig.h"

#include <optional>

class ElectrificationAltNodeNameChooser;
using ElectrificationNodeNameChooserPtr = std::shared_ptr<ElectrificationAltNodeNameChooser>;
using ElectrificationNodeNameChooserLink = std::weak_ptr<ElectrificationAltNodeNameChooser>;

/**
 * Класс-подборщик имен нод для компонентов электрической сети.
 * Класс создается на основе пресета альтернативного формата и используется
 * классами-генераторами.
 * Все методы класса принимают на входе блок контекстных данных критериев подбора (для конкретного компонента,
 * с заданной позицией) и выдают на выходе полный путь к модели ноды относительно папки ассетов data,
 * либо пустое значение, если модель с заданными критериями подобрать не удалось. Некоторые методы, также
 * выдают дополнительную информацию, такую как высота или ширина компонента (для опор или ригелей).
 *
 * Описание входны и выходных структур смотрите в заголовочном файле:
 * neokon-shared/electrification/ElectrificationContextDataTypes.h
 */
class ElectrificationAltNodeNameChooser
{
public:
    ElectrificationAltNodeNameChooser(ElectrificationMeshGeneratorConfigPtr const& config);

    /**
     * Подбор модели опоры на перегоне.
     */
    std::optional<std::string> choosePoleNodeName(
        electrification::PoleContextData const& contextData) const;

    /**
     * Подбор модели подвеса (консольной или фиксаторной стойки) под жесткой поперечиной.
     */
    std::optional<std::string> choosePortalSuspensionNodeName(
        electrification::PoleContextData const& contextData) const;

    /**
     * Подбор модели анкерной оттяжки. 
     */
    std::optional<std::string> chooseAnchorBackTieNodeName(
        electrification::AnchorContextData const& contextData) const;

    /**
     * Подбор модели анкерного блока.
     */
    std::optional<std::string> chooseAnchorBlockNodeName(
        electrification::AnchorContextData const& contextData) const;

    /**
     * Подбор имени нод прикрепляемых элементов подвески (струн, перемычек)
     */
    std::optional<std::string> chooseCatenaryAttachableNodeName(
        electrification::CatenaryAttachableType attachableType,
        electrification::CatenaryAttachableLength attachableLengthType) const;

    /**
     * Подбор модели левой боковой опоры жесткой поперечины.
     */
    std::optional<std::string> chooseLeftPortalPoleNodeName(
        electrification::PortalContextData const& contextData) const;

    /**
     * Подбор модели правой боковой опоры жесткой поперечины.
     */
    std::optional<std::string> chooseRightPortalPoleNodeName(
        electrification::PortalContextData const& contextData) const;

    /**
     * Подбор модели фермы (ригеля) жесткой поперечины. Возвращает
     * путь к ноде, а также реальную ширину в метрах (может отличаться от запрошенной в большую сторону).
     */
    std::optional<electrification::PortalPartChoosingResult>
        choosePortalCrossbarNodeName(electrification::PortalContextData const& contextData) const;

private:
    ElectrificationMeshGeneratorConfigPtr m_config;

    std::vector<int> m_cachedRigelAvailableLengths;
};
