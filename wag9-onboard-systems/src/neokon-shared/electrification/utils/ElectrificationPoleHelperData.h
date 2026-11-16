#pragma once

#include <UnigineMathLib.h>

#include <string>
#include <optional>
#include <unordered_map>

namespace electrification
{
struct PoleCantileverConnectors
{
public:
    Unigine::Math::Mat4 cable;
    std::optional<Unigine::Math::Mat4> wireFar;
    std::optional<Unigine::Math::Mat4> wireNear;
};

struct PoleSourceNodeHelperData
{
public:
    /**
     * key - connector node name (i.e 'nt', 'kp_2ws', 'kp_1wa' etc.),
     * value - local matrix within source node
     */
    std::unordered_map<std::string, Unigine::Math::Mat4> connectorsByNames;

    /**
     * Connector sets grouped by cantilevers. Key - name of Nt connector for cantilever.
     */
    std::unordered_map<std::string, PoleCantileverConnectors> connectorByCantilevers;

    /**
     * Mount point for anchor block and back tie node.
     */
    std::optional<Unigine::Math::Mat4> tensioningMountPoint;

    /**
     * Portal node mount points for portal crossbar
     */
    std::optional<Unigine::Math::Mat4> crossbarMountPointLeft;
    std::optional<Unigine::Math::Mat4> crossbarMountPointRight;

    //TODO: deprecated, used by old electrification module
    std::unordered_map<int, Unigine::Math::Mat4> connectorLocalBases;
};

//Основные коннекторы, встречающиеся на опорах.
extern const char* POLE_CONNECTOR_ID_NT;
extern const char* POLE_CONNECTOR_ID_KP_1WS;
extern const char* POLE_CONNECTOR_ID_KP_1WA;
extern const char* POLE_CONNECTOR_ID_KP_1WN;
extern const char* POLE_CONNECTOR_ID_KP_1WC;
extern const char* POLE_CONNECTOR_ID_KP_2WS;
extern const char* POLE_CONNECTOR_ID_KP_2WA;
extern const char* POLE_CONNECTOR_ID_KP_2WN;
extern const char* POLE_CONNECTOR_ID_KP_2WC;

//Коннекторы для переходных опор типа А и В,
//имеющих спаренные консоли для анкерной и переходной ветки.
extern const char* POLE_CONNECTOR_ID_NT_WS;
extern const char* POLE_CONNECTOR_ID_NT_WA;
extern const char* POLE_CONNECTOR_ID_NT_WN;
extern const char* POLE_CONNECTOR_ID_NT_WC;

//Переходные опоры типа С имеют по паре консолей - левую и правую,
//поддерживающих две паралелельные подвески.
//Подвеска, ближайшая к опоре цепляется к левой консоли, та, что дальше
//к правой.
extern const char* POLE_CONNECTOR_ID_KP_1WS_L;
extern const char* POLE_CONNECTOR_ID_KP_1WS_R;
extern const char* POLE_CONNECTOR_ID_KP_2WS_L;
extern const char* POLE_CONNECTOR_ID_KP_2WS_R;

extern const char* POLE_CONNECTOR_ID_NT_WS_L;
extern const char* POLE_CONNECTOR_ID_NT_WS_R;

//Коннекторы фиксаторного троса
extern const char* POLE_CONNECTOR_ID_FT;
extern const char* POLE_CONNECTOR_ID_TFT;
extern const char* POLE_CONNECTOR_ID_BFT;

extern const char* POLE_CONNECTOR_ID_CROSS_INS;
extern const char* SUSPENSION_CONNECTOR_SPN;

//Коннекторы изоляторов
extern const char* INSULATOR_CONNECTOR_NT_IN;
extern const char* INSULATOR_CONNECTOR_NT_OUT;
extern const char* INSULATOR_CONNECTOR_KP_1WS_IN;
extern const char* INSULATOR_CONNECTOR_KP_1WS_OUT;
extern const char* INSULATOR_CONNECTOR_KP_2WS_IN;
extern const char* INSULATOR_CONNECTOR_KP_2WS_OUT;
extern const char* INSULATOR_CONNECTOR_KP_2WS_A_IN;
extern const char* INSULATOR_CONNECTOR_KP_2WS_A_OUT;
extern const char* INSULATOR_CONNECTOR_KP_2WS_B_IN;
extern const char* INSULATOR_CONNECTOR_KP_2WS_B_OUT;
extern const char* ATTACHABLE_CONNECTOR_CROSS_WIRE_1;
extern const char* ATTACHABLE_CONNECTOR_CROSS_WIRE_2;
extern const char* ATTACHABLE_CONNECTOR_DROPPER;

//Коннекторы поперечины
extern const char* CROSSBAR_CONNECTOR_RIGHT;
extern const char* CROSSBAR_CONNECTOR_LEFT;

/**
 * Does nothing if the node with path @p nodePath has already been loaded.
 *
 * Must be called in the main thread.
 */
void fillPoleSourceNodeHelperData(std::string const& nodePath);

std::optional<PoleSourceNodeHelperData> findPoleNodeHelperData(std::string const& nodePath);

} // namespace electrification
