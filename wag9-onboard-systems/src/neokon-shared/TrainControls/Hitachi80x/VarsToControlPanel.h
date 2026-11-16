#pragma once

#include "neokon-shared/TrainControls/ControlPanelSerialization.h"
#include "neokon-shared/TrainControls/Hitachi80x/DriverDeskVersion.h"

namespace Hitachi
{

#pragma pack(push, 1)
struct VarsToControlPanel
{
    static inline DriverDeskVersion driverDeskVersion = DriverDeskVersion::Euston;

    bool lineVoltLamp = false;
    bool vcbLamp = false;

    float pressureGRBar = 0.f;
    float pressureTCBar = 0.f;
    float trainSpeedMpH = 0.f;
    float percents = 0.f;
    bool redTailSignalLeft = false;
    bool redTailSignalRight = false;
    bool whiteDownLeft = false;
    bool rearPreparationButton = false;
    bool trainWashLed = false;
    bool frontPreparationLed = false;
    bool draButtonLed = false;
    bool electricLed = false;
    bool engineStartLed = false;
    bool dieselLed = false;
    bool lampOtmr = false;
    bool onlyDoorLeft = false;
    bool onlyDoorRight = false;
    bool sleepModeLed = false;
    bool searchLightMarker = false;
    bool stopDieselLed = false;
    bool safetySystemIsolated = false;
    bool addLed = false;
    bool redFrontRightLed = false;
    bool redFrontLeftLed = false;
    bool whiteFrontLeftLed = false;
    bool whiteFrontRightLed = false;
    bool headLightRight = false;
    bool headLightLeft = false;
    bool hazardLightLed = false;
    bool snowBrakeLed = false;
    bool demisterLed = false;
    bool coupleLed = false;
    bool trainFaultAcknowledgeLed = false;
    bool displayYellowLed = false;
    bool displayRedLed = false;
    bool displayGreenLed = false;
    bool cctvButtonLed = false;
    bool eCouplerIsolation = false;

    bool tempIsolationFaultLit = false;
    bool spadLit = false;
    bool overspeedLit = false;
    bool trainStopOverrideLit = false;
    bool awsSunflowerLit = false;
    bool awsLit = false;

    bool doorReleaseLeftLit = false;
    bool doorReleaseRightLit = false;

    bool doorReleaseLeftBackPanelLit = false;
    bool doorReleaseRightBackPanelLit = false;

    bool doorInterlockLit = false;

    bool doorInterlockLeftBackPanelLit = false;
    bool doorInterlockRightBackPanelLit = false;

    bool actualMasterKey = false;
    int actualController = 0;

    bool PantControl = false;
    bool PantControlValve = false;
    bool PantADD = false;
    bool VCBControl1 = false;
    bool VCBControl2 = false;
    bool APC = false;
    bool CPBCControl1 = false;
    bool CPBCControl2 = false;
    bool CPBCControl3 = false;
    bool BrakeControl5 = false;
    bool BrakeSpeedSignal = false;
    bool CABHVACMain = false;
    bool ETCS = false;
    bool ATPControl = false;
    bool ATPDisplay = false;
    bool TPWSAWS = false;
    bool GSMR = false;
    bool DSD = false;
    bool EmerBrakeValve = false;
    bool EmerBrakeAlarm = false;
    bool ChangeOverControl = false;
    bool DoorControl = false;
    bool DoorControlPanel2 = false;
    bool DoorInd2 = false;
    bool Sanding = false;
    bool SpeedSignalRelay = false;
    bool LocoHauledMode = false;
    bool BatLowVoltage = false;
    bool CoupleControl = false;
    bool FrontHatch = false;
    bool DeskLamps = false;
    bool TMS1 = false;
    bool TMS2 = false;
    bool TMSMonitor = false;
    bool LeftExtLights = false;
    bool RightExtLights = false;
    bool extLightPanel = false;
    bool Wiper = false;
    bool TCA = false;
    bool EnergyMeter = false;
    bool CabSocketSub = false;
    bool CABHVACControl = false;
    bool CabVent = false;
    bool ExtStepLights = false;
    bool CabEmerLights = false;
    bool CabDoorClose = false;
    bool CabCelinigLight = false;
    bool EmerLightsControl = false;
    bool DSDPedal = false;
    bool CMD = false;
    bool leftLocalDoorOnly = false;
    bool rightLocalDoorOnly = false;
    bool wheelSlipSlideLed = false;
    bool trainDoorControlLeft = false;
    bool trainDoorControlRight = false;

    int toVirtualViewerWipersSpeedHz = 0;

    packet_io::Packet serialize() const;
    void deserialize(const packet_io::Packet& packet);
};
#pragma pack(pop)
} // namespace Hitachi
