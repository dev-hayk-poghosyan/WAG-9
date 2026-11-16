#pragma once

#include "neokon-shared/TrainControls/ControlPanelSerialization.h"


namespace Class170
{

enum class BISPositions : uint8_t
{
    AuxPlusStart = 0,
    Aux = 1,
    Isolate = 2,
    Start = 3,
};

enum class DDSPositions : uint8_t
{
    Off,
    Forward,
    Neutral,
    Revers,
};

#pragma pack(push, 1)
struct Controls
{
    bool dieselStopButton = false;
    bool dieselStartButton = false;
    bool DRA = false;
    bool washer = false;

    bool pedalDSD = false;
    bool holdoverDSD = false;

    bool hornLow = false;
    bool hornHigh = false;
    bool hornSoftLow = false;
    bool hornSoftHigh = false;

    bool emergencyStopPlunger = false;

    bool egressPedal = false;

    bool trainStopOverride = false;
    int tpwsTemporaryIsolation = 0;

    bool coupleButton = false;
    bool uncoupleButton = false;

    bool doorReleaseLeft1 = false;
    bool doorReleaseLeft2 = false;
    bool doorReleaseRight1 = false;
    bool doorReleaseRight2 = false;
    bool doorCloseInterlockRightSide = false;
    bool doorClose = false;
    bool doorCloseConductor = false;
    bool doorDeSelectConductor = false;
    bool signalConductor = false;
    bool doorRelease1Conductor = false;
    bool doorRelease2Conductor = false;
    bool signalButton = false;

    bool sandApply = false;

    int wipers = 0;
    float wipersHz = 0.f;

    bool sbTsa = false;
    bool sbFat = false;

    bool hazardLightsButton = false;

    int tailLightsSwitcher = 0;
    int lightsControl = 0;

    bool mainButtonAWS = false;

    DDSPositions DDS = DDSPositions::Off;
    int throttle = 0;
    bool hillButton = false;

    //Isolators
    bool switchDsdIsolation = false;
    bool switchAwsIsolation = false;
    bool switchVigilanceIsolation = false;
    bool switchTractionInterlockIsolation = false;
    bool switchPassComIsolation = false;
    bool switchDRAIsolation = false;
    bool switchEmergencyBrakeSupplyIsolation = false;
    bool switchFireAlarmIsolation = false;

    //From FS:
    BISPositions BIS = BISPositions::AuxPlusStart;

    bool leftSideCabDoorOpen = false;
    bool rightSideCabDoorOpen = false;

    bool suspensionSurgeTanksNo1Cock = false;
    bool suspensionSurgeTanksNo2Cock = false;

    bool chargeSocket1 = false;
    bool chargeSocket2 = false;

    bool dieselIsolated = false;
    bool localDieselStart = false;
    bool localDieselStop = false;

    bool mcbTCA = true;
    bool mcbAWS = true;
    bool mcbWiper1 = true;
    bool mcbWiper2 = true;
    bool mcbTrainControl = true;
    bool mcbBrakeControl = true;
    bool mcbDoorControl = true;
    bool mcbEndLights1 = true;
    bool mcbEndLights2 = true;

    bool mcbFaultProtect = true;

    bool firePull = false;
    bool fuelSupplyCock = false;
    bool engineStopButton = false;

    bool batteryChargingSocket1 = false;
    bool batteryChargingSocket2 = false;

    float speedMph = 0.f;
    float pressureBC = 0.f;
    float pressureMR = 0.f;

    bool deselectLeftButton = false;
    bool deselectRightButton = false;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct DataToDesk
{
    uint8_t trainRecorder = 0;
    uint8_t localFault = 0;
    uint8_t draggingBrakeFault = 0;
    uint8_t transmissionFault = 0;
    uint8_t parkingBrakeOnFault = 0;
    uint8_t TCAFault = 0;
    uint8_t engineStopFault = 0;
    uint8_t trainFault = 0;
    uint8_t doorDeselect = 0;
    uint8_t noTrainFault = 0;
    uint8_t WSP = 0;
    uint8_t doorPower = 0;
    uint8_t sunflower = 0;
    uint8_t DRA = 0;
    uint8_t brakeDemandLed = 0;
    uint8_t tempIsolationTPWSLed = 0;
    uint8_t trainStopOverrideLed = 0;
    uint8_t washerLed = 0;
    uint8_t doorRelease1LeftLed = 0;
    uint8_t doorInterlockLed = 0;
    uint8_t hazardLightsLed = 0;
    uint8_t passComAlarm = 0;
    uint8_t doorInterlockLostLed = 0;
    uint8_t reserved23 = 0;
    uint8_t demisterLed = 0;
    uint8_t roofLed = 0;
    uint8_t driverSideW = 0;
    uint8_t driverSideE = 0;
    uint8_t driverSideR = 0;
    uint8_t nonDriverSideW = 0;
    uint8_t nonDriverSideE = 0;
    uint8_t nonDriverSideR = 0;
    uint8_t doorDeselectConductor = 0;
    uint8_t signalConductor = 0;
    uint8_t doorReleaseConductor1 = 0;
    uint8_t doorReleaseConductor2 = 0;
    uint8_t doorCloseConductor = 0;
    uint8_t exitRightLed = 0;
    uint8_t doorReleaseRightLed = 0;
    uint8_t uncoupleLed = 0;
    uint8_t doorDeSelectRightLed = 0;
    uint8_t coupleLed = 0;
    uint8_t exitLeftLed = 0;
    uint8_t safetySystemIsolatedLed = 0;
    uint8_t doorCloseInterlockLeftLed = 0;
    uint8_t doorRelease2LeftLed = 0;
    uint8_t engineStartLed = 0;
    uint8_t applySandIntractionLed = 0;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct VirtualViewerIncomingVars
{
    DataToDesk todesk;
    Controls controls;

    packet_io::Packet serialize() const;
    bool deserialize(const packet_io::Packet& packet);
};
#pragma pack(pop)

static_assert(sizeof(Controls)   == 105);
static_assert(sizeof(DataToDesk) == 48);
static_assert(sizeof(VirtualViewerIncomingVars) == sizeof(Controls) + sizeof(DataToDesk));

} // namespace Class170
