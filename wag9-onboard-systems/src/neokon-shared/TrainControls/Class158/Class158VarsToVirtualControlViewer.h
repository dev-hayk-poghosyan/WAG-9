#pragma once

#include "neokon-shared/TrainControls/ControlPanelSerialization.h"
#include <stdint.h>

namespace Class158
{

enum class ControllerVersion
{
    ConsoleEmr,
};

enum class ThrustController : uint8_t
{
    Off = 0,
    Thrust1,
    Thrust2,
    Thrust3,
    Thrust4,
    Thrust5,
    Thrust6,
    Thrust7,
};

enum class DDSPositions : uint8_t
{
    Off,
    Forward,
    Neutral,
    Revers,
};

enum class BrakeController : uint8_t
{
    Release,
    Stage1,
    Stage2,
    FullService,
    Emergency,
};

/**
 * Values clockwise on the panel.
 */
enum class LightPosition : uint8_t
{
    Off,
    DayHeadLight,
    Off1,
    TailOnly,
    Off2,
    MarkerOnly,
    Hazard,
    NightHeadLight,
};

enum class TristateLight : uint8_t
{
    Off,
    White,
    Red,
};

#pragma pack(push, 1)
struct Controls
{
    bool engineStartButton = false;
    bool engineStopButton = false;
    bool mainButtonAWS = false;
    bool trainStopOverride = false;
    bool pedalDSD = false;

    bool masterKey = 0;
    DDSPositions dds = DDSPositions::Off;
    ThrustController controller = ThrustController::Off;
    BrakeController brakeController = BrakeController::Release;

    int tpwsTemporaryIsolation = 0;

    bool hornSoftHigh = false;
    bool hornHigh = false;

    bool coupleButton = false;
    bool uncoupleButton = false;

    bool signalButton = false;
    bool sander = false;

    bool switchCompressorSpeedUp = false;
    bool switchDestIndicator = false;
    bool switchInstrumentLights = false;
    bool switchDraIsolate = false;

    LightPosition lights = LightPosition::Off;

    bool dra = false;
    bool demist = false;

    bool deserialize(const packet_io::Packet& packet);
};

struct DataToDesk
{
    bool engineStop = false;
    bool brakeDemandLed = false;
    bool trainStopOverrideLed = false;
    bool tempIsolationTPWSLed = false;
    bool sunflower = false;

    bool draLed = false;

    bool unitFaultLed = false;
    bool trainFaultLed = false;
    bool safetySystemIsolatedLed = false;
    bool engineStopLed = false;

    bool coupleLed = false;
    bool uncoupleLed = false;

    /**
     * Perspective of the driver.
     */
    TristateLight headLightRight = TristateLight::Off;
    TristateLight headLightLeft  = TristateLight::Off;
    TristateLight tailLightRight = TristateLight::Off;
    TristateLight tailLightLeft  = TristateLight::Off;

    /**
     * Value of 0..10.0f expected.
     */
    float mr = 0.f;

    /**
     * Value of [0..7.0f] expected.
     */
    float bc = 0.f;

    /**
     * Value of [0..120] expected.
     */
    uint8_t speedMph = 0;

    std::vector<uint8_t> serialize() const;
};

struct VirtualViewerIncomingVars
{
    DataToDesk todesk;
    Controls controls;

    packet_io::Packet serialize() const;
    bool deserialize(const packet_io::Packet& packet);
};
#pragma pack(pop)

static_assert(sizeof(Controls)   == 26);
static_assert(sizeof(DataToDesk) == 25);
static_assert(sizeof(VirtualViewerIncomingVars) == sizeof(Controls) + sizeof(DataToDesk));

} // namespace Class158
