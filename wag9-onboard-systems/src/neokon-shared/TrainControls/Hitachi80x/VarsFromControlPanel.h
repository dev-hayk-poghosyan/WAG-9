#pragma once
#include <cstdint>

#include "neokon-shared/TrainControls/Hitachi80x/DriverDeskVersion.h"

namespace Hitachi
{

enum class DriverDirectionSwitch
{
    Off,
    Forward,
    Neutral,
    Reverse,
};


enum class TpwsTemporaryIsolation
{
    None,
    Normal,
    Isolated
};

#pragma pack(push, 1)
struct VarsFromControlPanel
{
    static inline DriverDeskVersion driverDeskVersion = DriverDeskVersion::Euston;

    int bufferLights = 0;

    bool trainCommunicationPA2 = false;
    bool coupleButton = false;
    bool uncoupleButton = false;
    bool trainFaultAcknowledge = false;

    int wipers = 0;
    int wipersSpeedHz = 0;
    bool windscreenDemist = false;
    bool washerEnabled = false;

    bool auxOnButton = false;
    bool auxOffButton = false;
    int masterKeySwitch = 0;
    DriverDirectionSwitch dds = DriverDirectionSwitch::Off;
    bool panUpButton = false;
    bool panDownButton = false;
    bool electricButton = false;
    bool typhon = false;
    bool whistle = false;
    bool rearPreparationButton = false;
    bool frontPreparationButton = false;
    bool trainWashButton = false;
    float throttleController = 0.f;
    bool engineStartButton = false;
    bool dieselButton = false;
    bool engineStopButton = false;
    bool draButton = false;
    bool sleepModeDeactivation = false;
    bool tmsReset = false;
    bool leftEmergencyButton = false;
    bool rightEmergencyButton = false;
    bool emergencyDieselStop = false;
    bool hazardLight = false;
    bool buttonCPBC = false;
    bool snowBrakeButton = false;
    bool eCouplerIsolation = false;
    bool emergencyLightsOff = false;
    bool locoHauledMode = false;
    bool batLowVoltage = false;
    bool coupleControlMcb = false;
    int unattnededSwitch = 0;
    bool pantographChanger = false;
    bool rescueMode = false;
    bool switcherEtcsBr = false;
    int airConController = 0;
    int tractionLimit = 0;

    bool etcsIso = false;
    bool tractionIso = false;
    bool sdoIsolation = false;
    bool brAtpIso = false;

    bool awsResetPressed = false;
    bool pedal = false;
    bool dsdPanelButton = false;
    bool trainStopOverride = false;
    bool signalFrontPanelLeft = false;
    bool signalFrontPanelRight = false;
    bool signalBackPanelRight = false;
    bool signalBackPanelLeft = false;
    TpwsTemporaryIsolation tpwsTemporaryIsolation = TpwsTemporaryIsolation::None;

    bool switchAwsIsolation = false;
    bool switchTpwsAwsIsolation = false;
    bool cabSocket = false;
    bool cabSocketSub = false;
    bool switchDsdIsolation = false;
    bool switchEtcsIsolation = false;
    bool switchTractionIntrlockIsolation = false;
    bool switchDraIsolation = false;
    bool switchSdoIsolation = false;
    bool switchEmergencyBrakeSuplyIsolation = false;
    bool switchPassCommIsolation = false;
    bool switchAddIsolation = false;
    bool switchVigilanceIsolation = false;
    bool switchBrAtpIsolation = false;

    bool overspeedPressed = false;
    bool awsPressed = false;
    bool spadPressed = false;
    bool brakeReleasePressed = false;
    bool unitHatchClose = false;

    bool dsdPeadlUp = false;
    bool dsdPeadlDown = false;

    bool footPedal = false;
    int hvac = 0;
    int heather = 0;
    bool docsLights = false;
    bool deskLights = false;
    bool downLighter = false;
    bool cabCentralLight = false;
    bool wheelSlipSlide = false;

    bool buttonCCTV = false;

    //Circuit breakers:

    bool cpbc1Mcb = false;
    bool cpbc2Mcb = false;
    bool cpbc3Mcb = false;
    bool tms1 = false;
    bool tms2 = false;
    bool tmsMonitor = false;
    bool leftExtLightsMcb = false;
    bool rightExtLightsMcb = false;
    bool extLightPanelMcb = false;
    bool wipersMcb = false;
    bool tcaMcb = false;
    bool energyMeterMcb = false;
    bool vcb1Mcb = false;
    bool vcb2Mcb = false;
    bool apcMcb = false;
    bool etcsMcb = true;
    bool cctvMcb = true;
    bool atpMcb = false;
    bool gsmrMcb = false;
    bool cbDsd = false;
    bool emerBrakeValveMcb = false;
    bool emerBrakeAlarmMcb = false;
    bool changeoverControlMcb = false;
    bool cbDsdPedal = false;
    bool cbTpwsAws = false;
    bool brakeControlMcb = false;
    bool brakeSpeedSignalMcb = false;
    bool atpDisplayMcb = false;
    bool cabHvacControlMcb = false;
    bool cabVentMcb = false;
    bool extStepLightsMcb = false;
    bool cabEmerLightsMcb = false;
    bool cabDoorCloseMcb = false;
    bool cabCeilingLightMcb = false;
    bool emerLightControlMcb = false;
    bool mcbFrontHatch = false;
    bool deskLapmsMcb = false;
    bool buzzerOnlyOnLeft = false;
    bool buzzerOnlyOnRight = false;
    bool cmdMcb = false;
    bool doorControlMcb = false;
    bool doorControlPanelMcb = false;
    bool doorIndication = false;
    bool sandingMcb = false;
    bool slipButton = false;
    bool unattended = false;
    bool speedSignalRelayMcb = false;
    bool buzzerOnlyOffLeft = false;
    bool buzzerOnlyOffRight = false;
    bool cctvButton = false;
    bool pantControl = false;
    bool pantControlValve = false;
    bool pantAdd = false;
    bool addIsolation = false;
    bool cabHyacMain = false;

    bool emergencyBrakeSupplyIso = false;
    bool passComIso = false;
    bool isoAdd = false;

    bool doorReleaseFrontLeftL = false;
    bool doorReleaseFrontLeftR = false;
    bool doorReleaseBackLeftL = false;
    bool doorReleaseBackLeftR = false;

    bool doorReleaseFrontRightL = false;
    bool doorReleaseFrontRightR = false;
    bool doorReleaseBackRightL = false;
    bool doorReleaseBackRightR = false;

    bool doorCloseInterlockFrontLeft = false;
    bool doorCloseInterlockFrontRight = false;
    bool doorCloseInterlockBackLeft = false;
    bool doorCloseInterlockBackRight = false;

    bool doorControlFrontRight = false;
    bool doorControlFrontLeft = false;
    bool doorControlBackRight = false;
    bool doorControlBackLeft = false;

    bool tempUp = false;
    bool tempDown = false;

    bool leftDoorControl = false;
    bool rightDoorControl = false;

    bool cctvManuallyHidden = false;

    void deserialize(uint8_t const* data, size_t size);
};
#pragma pack(pop)
}
