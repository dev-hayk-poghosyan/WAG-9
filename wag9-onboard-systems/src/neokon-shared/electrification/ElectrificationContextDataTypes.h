#pragma once

#include "ElectrificationSetEnumTypes.h"
#include "ElectrificationPole.pb.h"
#include <UnigineMathLib.h>

#include <optional>

/**
 * Данный заголовочный файл содержит контекстные блоки данных -
 * то есть свойств уникальных для каждого экземпляра элемента
 * контактной сети, таких как позиция и трансформация опоры,
 * или тип фиксатора на консоли. Контекстные данные используются
 * совместно с данными из пресетов при генерации отдельных составляющих
 * участка КС.
 *
 * TODO: на данный момент в структурах представлен не весь набор
 * контекстных данных, необходимый классам-генераторам, а лишь его подможество
 * нужное для работы класса ElectrificationNodeNameChooser.
 */
namespace electrification
{
/**
 *  Тип опоры в контексте участка пути.
 */
enum class PoleType : int
{
    Middle = proto::electrification::Pole_Type_MIDDLE, //промежуточная опора участка КС
    Transitional = proto::electrification::Pole_Type_TRANSITIONAL, //переходная опора сопряжения
    PortalPillar = proto::electrification::Pole_Type_PORTAL_PILLAR //боковая опора жесткой поперечины
};

/**
 *  Подтип для промежуточных опор сопряжения.
 */
enum class TransitionalPoleType : int
{
    TransitionalA = proto::electrification::Pole_TransitionalType_TRANSITIONAL_A, //первая опора сопряжения
    TransitionalB = proto::electrification::Pole_TransitionalType_TRANSITIONAL_B, //последняя опора сопряжения
    TransitionalC = proto::electrification::Pole_TransitionalType_TRANSITIONAL_C, //средняя опора 4-пролетного сопряжения
    TransitionalAInsulated = proto::electrification::Pole_TransitionalType_TRANSITIONAL_A_INSULATED, //первая опора изолированного сопряжения
    TransitionalBInsulated = proto::electrification::Pole_TransitionalType_TRANSITIONAL_B_INSULATED, //последняя опора изолированного сопряжения
    TransitionalCInsulated = proto::electrification::Pole_TransitionalType_TRANSITIONAL_C_INSULATED //средняя опора 4-пролетного изолированного сопряжения
};

/**
 *  Тип фиксатора контактного провода.
 */
enum class RegistrationArmType : int
{
    StraightPullOff = proto::electrification::RegistrationArm_Type_STRAIGHT_PULL_OFF, //прямой фиксатор для прямого пути
    StraightPushOff = proto::electrification::RegistrationArm_Type_STRAIGHT_PUSH_OFF, //обратный фиксатор для прямого пути
    StraightJunction = proto::electrification::RegistrationArm_Type_STRAIGHT_JUNCTION, //опора воздушной стрелки
    CurvePullOff = proto::electrification::RegistrationArm_Type_CURVE_PULL_OFF, //прямой фиксатор для установки с внешней стороны кривой
    CurvePushOff = proto::electrification::RegistrationArm_Type_CURVE_PUSH_OFF, //обратный фиксатор для установки на внутренней стороне кривой
    CurveJunction = proto::electrification::RegistrationArm_Type_CURVE_JUNCTION //опора воздушной стрелки в кривой (оба фиксатора однонаправленные)
};

/**
 *  Блок контекстных данных для выбора модели опоры на участке КС.
 */
struct PoleContextData
{
    Unigine::Math::Mat4 transform;
    ConsoleType         consoleType = ConsoleType::Isolated;
    PoleType            poleType = PoleType::Middle;
    RegistrationArmType registrationArmType = RegistrationArmType::StraightPullOff;
    std::optional<TransitionalPoleType> transitionalPoleSubtype;
    std::optional<PortalPillarSide> portalPillarSide;
    std::optional<std::string> nodeName; //Нода опоры: если назначено, используется вместо
                                         //стандартных нод пресета
    bool isSuspension{ false };
    bool flipRegistrationArmType{ false };

    bool operator==(PoleContextData const& other) const
    {
        if (registrationArmType != other.registrationArmType)
        {
            return false;
        }

        if (poleType != other.poleType)
        {
            return false;
        }

        if (transitionalPoleSubtype != other.transitionalPoleSubtype)
        {
            return false;
        }

        if (isSuspension != other.isSuspension)
        {
            return false;
        }

        return true;
    }

    bool operator!=(PoleContextData const& other) const
    {
        return !(operator==(other));
    }
};

/**
 * Блок контекстных данных для анкеровки (компенсационных устройств).
 */
struct AnchorContextData
{
    TensioningType tensioningType;
    /**
     * Средняя анкеровка использует свой отдельный
     * набор анкерных оттяжек и блоков. Если middleTensioning
     * установлено в true, значение tensioningType при выборе ноды
     * блока или оттяжки будет проигнорировано. 
     */
    bool middleTensioning{ false };
};

/*
 * Блок контекстных данных для жестких поперечин или
 * отдельных блоков жестких поперечин (ферм).
 */
struct PortalContextData
{
    Unigine::Math::Mat4 transform;
    double desiredLength{ 0.0 };
    std::set<double> suspensionDistances;
    /**
     * Suspension dropper connectors' positions. Only used for flex portals
     */
    std::vector<Unigine::Math::Vec3> suspensionDropperConnectors;
};

/**
 * Блок контекстных данных для одной из опор, соединённых проводом вручную
 */

struct ManualWireContextData
{
    int objectId{ -1 };
    std::string cantileverGroup;
    CatenaryType catenaryType{ CatenaryType::Simple };
    TensioningType tensioningType{TensioningType::None};
};

/**
 * Структура, возвращаемая как результат алогритма
 * подбора элемента жесткой поперечины (фермы-ригеля или мостика).
 * Подобные элементы могут иметь длину, отличную от заявленной в
 * PortalContextData::desiredLength
 */
struct PortalPartChoosingResult
{
public:
    std::string path;
    double actualLength{ 0 };
};

/**
 * Блок контекстных данных для стоек жестких поперчин.
 */
struct PortalPillarContextData
{
    PortalPillarSide pillarSide;
};

enum class CatenaryAttachableType
{
    Dropper,
    Dropper1w,
    InsulatorDropper,
    InsulatorDropper1w,
    CompensatedMidpointAnchor,
    SemiCompensatedMidpointAnchor,
    SpanWireConnector,
};

enum class CatenaryAttachableLength : int
{
    Short = 0,
    Medium,
    Long,
};

using CatenaryAttachableNodeName = std::map<CatenaryAttachableLength, std::string>;

enum class InsulatorPositionType : int
{
    SectionUpper,
    SectionLower,
    TensionUpper,
    TensionLower,
    OverlapUpper,
    OverlapLower,
    Reg,
};


using ConnectorId = int;
struct CatenaryWireData
{
    Unigine::Math::Vec3 startPosition;
    Unigine::Math::Vec3 endPosition;
};
/*
 * Блок контекстных данных для подвесок.
 * TODO: предполагается, что подвески будут целиком генерироваться,
 * но на данный момент существует возможность выбора моделей, заготовленных
 * художниками. И для тех и для других нужны контекстные данные - как минимум
 * трансформации точек крепления. Полный набор контекстных данных будет прояснен
 * в ходе реализации классов-генераторов подвесок.
 */
struct CatenaryContextData
{
    bool isAnchor{ false };

    std::unordered_map<ConnectorId, CatenaryWireData> contactWires;
    CatenaryWireData messengerWire;

    bool leftStitch{ false };
    bool rightStitch{ false };

    CatenaryAttachableNodeName dropperNodeName;
    CatenaryAttachableNodeName dropperNodeName1w;
    CatenaryAttachableNodeName insulatorDropperNodeName;
    CatenaryAttachableNodeName insulatorDropperNodeName1w;
    CatenaryAttachableNodeName compMidPointAnchorName;
    CatenaryAttachableNodeName scompMidPointAnchorName;
    CatenaryAttachableNodeName spanWireConnectorNodeName;
};

inline RegistrationArmType
registrationArmTypeFromProto(proto::electrification::RegistrationArm::Type const& value)
{
    static_assert(+RegistrationArmType::StraightPullOff ==
        +proto::electrification::RegistrationArm_Type_STRAIGHT_PULL_OFF);
    static_assert(+RegistrationArmType::StraightPushOff ==
        +proto::electrification::RegistrationArm_Type_STRAIGHT_PUSH_OFF);
    static_assert(+RegistrationArmType::StraightJunction ==
        +proto::electrification::RegistrationArm_Type_STRAIGHT_JUNCTION);
    static_assert(+RegistrationArmType::CurvePullOff ==
        +proto::electrification::RegistrationArm_Type_CURVE_PULL_OFF);
    static_assert(+RegistrationArmType::CurvePushOff ==
        +proto::electrification::RegistrationArm_Type_CURVE_PUSH_OFF);
    static_assert(+RegistrationArmType::CurveJunction ==
        +proto::electrification::RegistrationArm_Type_CURVE_JUNCTION);

    return static_cast<RegistrationArmType>(value);
}

inline proto::electrification::RegistrationArm::Type
registrationArmTypeToProto(RegistrationArmType const& value)
{
    static_assert(+RegistrationArmType::StraightPullOff ==
                  +proto::electrification::RegistrationArm_Type_STRAIGHT_PULL_OFF);
    static_assert(+RegistrationArmType::StraightPushOff ==
                  +proto::electrification::RegistrationArm_Type_STRAIGHT_PUSH_OFF);
    static_assert(+RegistrationArmType::StraightJunction ==
                  +proto::electrification::RegistrationArm_Type_STRAIGHT_JUNCTION);
    static_assert(+RegistrationArmType::CurvePullOff ==
                  +proto::electrification::RegistrationArm_Type_CURVE_PULL_OFF);
    static_assert(+RegistrationArmType::CurvePushOff ==
                  +proto::electrification::RegistrationArm_Type_CURVE_PUSH_OFF);
    static_assert(+RegistrationArmType::CurveJunction ==
                  +proto::electrification::RegistrationArm_Type_CURVE_JUNCTION);

    return static_cast<proto::electrification::RegistrationArm::Type>(value);
}

inline PoleType
poleTypeFromProto(proto::electrification::Pole::Type const& value)
{
    static_assert(+PoleType::Middle == +proto::electrification::Pole_Type_MIDDLE);
    static_assert(+PoleType::Transitional == +proto::electrification::Pole_Type_TRANSITIONAL);
    static_assert(+PoleType::PortalPillar == +proto::electrification::Pole_Type_PORTAL_PILLAR);

    return static_cast<PoleType>(value);
}

inline proto::electrification::Pole::Type
poleTypeToProto(PoleType const& value)
{
    static_assert(+PoleType::Middle == +proto::electrification::Pole_Type_MIDDLE);
    static_assert(+PoleType::Transitional == +proto::electrification::Pole_Type_TRANSITIONAL);
    static_assert(+PoleType::PortalPillar == +proto::electrification::Pole_Type_PORTAL_PILLAR);

    return static_cast<proto::electrification::Pole::Type>(value);
}

inline TransitionalPoleType
transitionalPoleTypeFromProto(proto::electrification::Pole::TransitionalType const& value)
{
    static_assert(+TransitionalPoleType::TransitionalA ==
        +proto::electrification::Pole_TransitionalType_TRANSITIONAL_A);
    static_assert(+TransitionalPoleType::TransitionalB ==
        +proto::electrification::Pole_TransitionalType_TRANSITIONAL_B);
    static_assert(+TransitionalPoleType::TransitionalC ==
        +proto::electrification::Pole_TransitionalType_TRANSITIONAL_C);
    static_assert(+TransitionalPoleType::TransitionalAInsulated ==
        +proto::electrification::Pole_TransitionalType_TRANSITIONAL_A_INSULATED);
    static_assert(+TransitionalPoleType::TransitionalBInsulated ==
        +proto::electrification::Pole_TransitionalType_TRANSITIONAL_B_INSULATED);
    static_assert(+TransitionalPoleType::TransitionalCInsulated ==
        +proto::electrification::Pole_TransitionalType_TRANSITIONAL_C_INSULATED);

    return static_cast<TransitionalPoleType>(value);
}

inline proto::electrification::Pole::TransitionalType
transitionalPoleTypeToProto(TransitionalPoleType const& value)
{
    static_assert(+TransitionalPoleType::TransitionalA ==
                  +proto::electrification::Pole_TransitionalType_TRANSITIONAL_A);
    static_assert(+TransitionalPoleType::TransitionalB ==
                  +proto::electrification::Pole_TransitionalType_TRANSITIONAL_B);
    static_assert(+TransitionalPoleType::TransitionalC ==
                  +proto::electrification::Pole_TransitionalType_TRANSITIONAL_C);
    static_assert(+TransitionalPoleType::TransitionalAInsulated ==
                  +proto::electrification::Pole_TransitionalType_TRANSITIONAL_A_INSULATED);
    static_assert(+TransitionalPoleType::TransitionalBInsulated ==
                  +proto::electrification::Pole_TransitionalType_TRANSITIONAL_B_INSULATED);
    static_assert(+TransitionalPoleType::TransitionalCInsulated ==
                  +proto::electrification::Pole_TransitionalType_TRANSITIONAL_C_INSULATED);

    return static_cast<proto::electrification::Pole::TransitionalType>(value);
}

}
