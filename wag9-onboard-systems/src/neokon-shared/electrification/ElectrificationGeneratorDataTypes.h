#pragma once

#include "neokon-shared/BezierSpline.h"
#include "neokon-shared/electrification/ElectrificationContextDataTypes.h"

namespace electrification
{
/**
 * Тип радиуса кривизны для двухпутных участков.
 * Влияет на тип и чередование фиксаторов на консолях жд-опор.
 */
enum class CurvatureSide : int
{
    None = 0, //Отсуствует (прямой путь)
    Inner,    //Путь на внутренней стороне круга.
    Outer     //Путь на внешней стороне закругления.
};

/**
 * Расчитанные данные для одной жд-опоры.
 * Поля actualIntervalM и signedCurveRadiusM содержат значения для пролетов.
 * Здесь подразумевается пролет между текущей опорой и предудыщей опорой в массиве,
 * который возвращает функция calculateDynamicPolePositions.
 */
struct DynamicPoleGenInfo : PoleContextData
{
    std::optional<int> customId; //ИД опоры, если это пользовательская опора
    std::string preset;      //Пресет для генерации
    double pathDistanceM{0}; //Дистанция в метрах от начала пути.
    double actualIntervalM{0}; //Длина пролета в метрах от предыдущей опоры.
    double signedCurveRadiusM{0}; //Радиус кривизны в метрах со знаком (+-).
                                  //Подробнее смотрите описание функции @calcBezierSplineCurveRadius
    CurvatureSide curvatureSide{CurvatureSide::None}; //Тип радиуса кривизны (внешний/внутренний).

    bool operator==(DynamicPoleGenInfo const& other) const
    {
        auto const& lhs = static_cast<PoleContextData const&>(*this);
        auto const& rhs = static_cast<PoleContextData const&>(other);

        if (rhs != lhs)
        {
            return false;
        }

        if (preset != other.preset)
        {
            return false;
        }

        if (nodeName != other.nodeName)
        {
            return false;
        }

        if (registrationArmType != other.registrationArmType)
        {
            return false;
        }

        if (flipRegistrationArmType != other.flipRegistrationArmType)
        {
            return false;
        }

        const double eps = 1e-6;
        if ((transform.getTranslate() - other.transform.getTranslate()).length2() > eps)
        {
            return false;
        }

        if ((transform.getAxisX() - other.transform.getAxisX()).length2() > eps)
        {
            return false;
        }

        if ((transform.getAxisY() - other.transform.getAxisY()).length2() > eps)
        {
            return false;
        }

        return true;
    }

    bool operator!=(DynamicPoleGenInfo const& other) const { return !(operator==(other)); }
};

/**
 * Рассчитанные данные для генерации одной жесткой поперечины.
 */
struct DynamicPortalGenInfo : PortalContextData
{
    std::string preset;
    int portalId{0};
};

struct SuspensionPositionInfo
{
    Unigine::Math::Mat4 suspensionTransform;
    std::string preset;
};

struct DynamicManualWireConnectInfo : ManualWireContextData
{
    electrification::DynamicPoleGenInfo poleGenInfo;
    Unigine::Math::Mat4 transform;

    bool operator==(DynamicManualWireConnectInfo const& other) const
    {
        if (objectId != other.objectId)
        {
            return false;
        }

        if (cantileverGroup != other.cantileverGroup)
        {
            return false;
        }

        if (tensioningType != other.tensioningType)
        {
            return false;
        }

        if (poleGenInfo != other.poleGenInfo)
        {
            return false;
        }

        const double eps = 1e-6;
        if ((transform.getTranslate() - other.transform.getTranslate()).length2() > eps)
        {
            return false;
        }

        if ((transform.getAxisX() - other.transform.getAxisX()).length2() > eps)
        {
            return false;
        }

        if ((transform.getAxisY() - other.transform.getAxisY()).length2() > eps)
        {
            return false;
        }

        return true;
    }

    bool operator!=(DynamicManualWireConnectInfo const& other) const
    {
        return !(operator==(other));
    }
};

struct DynamicManualWireGenInfo
{
    DynamicManualWireConnectInfo left;
    DynamicManualWireConnectInfo right;

    bool operator==(DynamicManualWireGenInfo const& other) const
    {
        if (left != other.left)
        {
            return false;
        }

        if (right != other.right)
        {
            return false;
        }

        return true;
    }

    bool operator!=(DynamicManualWireGenInfo const& other) const { return !(operator==(other)); }
};

enum class CreationMode
{
    NODES,
    CLUSTER
};

/**
 * Базовый тип данных для всех генераторов опор
 */
struct PoleGenerationArgs
{
    CreationMode creationMode{ CreationMode::CLUSTER };
    AlignedBox<glm::dvec3> aabb; // Берётся из сплайна
    std::string defaultCantileverPreset;
    CatenaryType catenaryType{ CatenaryType::Simple }; //Тип подвески: рессорная/безрессорная.
    TensionPathPlacement tensionPathPlacement{ TensionPathPlacement::None };
    bool generateWires{ true };

    std::vector<DynamicPoleGenInfo> poleGenInfo;
};

/**
 * Входные параметры для генератора жестких поперечин.
 */
struct PortalGenerationArgs
{
    //Способ создания - нодами или через кластеры.
    CreationMode creationMode{CreationMode::CLUSTER};
    //Ограничительный куб, используется классом-менеджером (хостом) генераторов. 
    AlignedBox<glm::dvec3> aabb;
    //Массив структур, описыаающих жесткие поперечины.
    std::list<DynamicPortalGenInfo> portalGenInfo;
};

struct ManualWireGenerationArgs
{
    //Способ создания - нодами или через кластеры.
    CreationMode creationMode{CreationMode::CLUSTER};
    //Ограничительный куб, используется классом-менеджером (хостом) генераторов.
    AlignedBox<glm::dvec3> aabb;
    //Массив структур, описыаающих кастомные провода
    std::list<DynamicManualWireGenInfo> wiresGenInfo;
};

} // namespace electrification
