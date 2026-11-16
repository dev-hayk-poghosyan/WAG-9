#include "neokon-shared/TrainControls/Hitachi80x/VarsToControlPanel.h"

namespace Hitachi
{

packet_io::Packet
VarsToControlPanel::serialize() const
{
    using namespace packet_io;
    size_t const packetSize = driverDeskVersion == DriverDeskVersion::Euston ? 124 :
        driverDeskVersion == DriverDeskVersion::Crewe ?
        115 : 127;

    Packet packet(packetSize);

    writeU16BE(packet.data(), 0, percents * 999.f / 150.f);
    writeU16BE(packet.data(), 2, pressureGRBar * 999.f / 12.f);
    writeU16BE(packet.data(), 4, pressureTCBar * 999.f / 12.f);

    if (driverDeskVersion == DriverDeskVersion::Euston)
    {
        writeU8(packet.data(), 6, searchLightMarker);
        writeU8(packet.data(), 7, redTailSignalLeft);
        writeU8(packet.data(), 8, redTailSignalRight);
        writeU8(packet.data(), 9, headLightLeft);
        writeU8(packet.data(), 10, redFrontLeftLed);
        writeU8(packet.data(), 11, headLightRight);
        writeU8(packet.data(), 12, redFrontRightLed);
        writeU8(packet.data(), 13, hazardLightLed);
        //writeU8(packet.data(), 14, hazardLightLed);   cctvEvent
        writeU8(packet.data(), 15, doorReleaseLeftLit);
        writeU8(packet.data(), 17, doorInterlockLit);
        //writeU8(packet.data(), 18, doorInterlockLit);  trainDoorControlLeft
        writeU8(packet.data(), 19, lineVoltLamp);
        writeU8(packet.data(), 20, safetySystemIsolated);
        writeU8(packet.data(), 21, addLed);
        writeU8(packet.data(), 22, snowBrakeLed);
        writeU8(packet.data(), 23, wheelSlipSlideLed);
        writeU8(packet.data(), 24, trainWashLed);
        writeU8(packet.data(), 25, vcbLamp);
        writeU8(packet.data(), 28, displayYellowLed);
        writeU8(packet.data(), 29, spadLit);
        writeU8(packet.data(), 30, overspeedLit);
        writeU8(packet.data(), 31, awsLit);
        writeU8(packet.data(), 32, tempIsolationFaultLit);
        writeU8(packet.data(), 33, trainStopOverrideLit);
        writeU8(packet.data(), 34, frontPreparationLed);
        writeU8(packet.data(), 35, coupleLed);
        writeU8(packet.data(), 36, rearPreparationButton);
        writeU8(packet.data(), 37, awsSunflowerLit);
        writeU8(packet.data(), 38, draButtonLed);
        writeU8(packet.data(), 40, doorInterlockLit);
        writeU8(packet.data(), 41, doorReleaseRightLit);
        writeU8(packet.data(), 43, electricLed);
        writeU8(packet.data(), 44, trainFaultAcknowledgeLed);
        writeU8(packet.data(), 45, demisterLed);
        writeU8(packet.data(), 46, engineStartLed);
        writeU8(packet.data(), 47, dieselLed);
        writeU8(packet.data(), 48, stopDieselLed);
        writeU8(packet.data(), 50, whiteFrontRightLed);
        writeU8(packet.data(), 51, CabCelinigLight);
        writeU8(packet.data(), 52, CabEmerLights);
        writeU8(packet.data(), 53, DSD);
        writeU8(packet.data(), 54, eCouplerIsolation);
        writeU8(packet.data(), 55, sleepModeLed);
        writeU8(packet.data(), 56, lampOtmr);
        writeU8(packet.data(), 57, doorInterlockRightBackPanelLit);
        writeU8(packet.data(), 58, rightLocalDoorOnly);
        writeU8(packet.data(), 59, doorReleaseRightBackPanelLit);
        writeU8(packet.data(), 60, doorReleaseRightBackPanelLit);
        writeU8(packet.data(), 61, doorInterlockLeftBackPanelLit);
        writeU8(packet.data(), 62, leftLocalDoorOnly);
        writeU8(packet.data(), 63, doorReleaseLeftBackPanelLit);
        writeU8(packet.data(), 64, doorReleaseLeftBackPanelLit);
        writeU8(packet.data(), 69, PantADD);
        writeU8(packet.data(), 70, PantControl);
        writeU8(packet.data(), 71, EmerBrakeAlarm);
        writeU8(packet.data(), 73, TPWSAWS);
        writeU8(packet.data(), 74, ATPControl);
        writeU8(packet.data(), 75, ETCS);
        writeU8(packet.data(), 77, EnergyMeter);
        writeU8(packet.data(), 78, Wiper);
        writeU8(packet.data(), 79, RightExtLights);
        writeU8(packet.data(), 80, TMS2);
        writeU8(packet.data(), 81, DeskLamps);
        writeU8(packet.data(), 82, LocoHauledMode);
        writeU8(packet.data(), 83, CoupleControl);
        writeU8(packet.data(), 84, Sanding);
        writeU8(packet.data(), 85, DoorControlPanel2);
        writeU8(packet.data(), 86, CMD);
        writeU8(packet.data(), 87, EmerLightsControl);
        writeU8(packet.data(), 88, CabDoorClose);
        writeU8(packet.data(), 89, ExtStepLights);
        writeU8(packet.data(), 90, CabVent || CABHVACControl);
        writeU8(packet.data(), 91, CabSocketSub);
        writeU8(packet.data(), 92, BrakeControl5);
        writeU8(packet.data(), 93, TMSMonitor);
        writeU8(packet.data(), 94, TMS1);
        writeU8(packet.data(), 95, FrontHatch);
        writeU8(packet.data(), 96, BatLowVoltage);
        writeU8(packet.data(), 97, SpeedSignalRelay);
        writeU8(packet.data(), 98, DoorInd2);
        writeU8(packet.data(), 100, DSDPedal);
        writeU8(packet.data(), 103, CPBCControl2);
        writeU8(packet.data(), 105, VCBControl1);
        writeU8(packet.data(), 106, PantControlValve);
        writeU8(packet.data(), 107, ChangeOverControl);
        writeU8(packet.data(), 108, EmerBrakeValve);
        writeU8(packet.data(), 109, GSMR);
        writeU8(packet.data(), 110, ATPDisplay);
        writeU8(packet.data(), 111, ATPControl);
        writeU8(packet.data(), 112, CABHVACMain);
        writeU8(packet.data(), 113, TCA);
        writeU8(packet.data(), 114, extLightPanel);
        writeU8(packet.data(), 115, LeftExtLights);
        writeU8(packet.data(), 116, CPBCControl3);
        writeU8(packet.data(), 117, CPBCControl1);
        writeU8(packet.data(), 118, VCBControl2);
        writeU8(packet.data(), 119, BrakeSpeedSignal);
        writeU8(packet.data(), 123, whiteFrontRightLed);
    }
    else if (driverDeskVersion == DriverDeskVersion::Crewe)
    {
        writeU8(packet.data(), 6, redTailSignalRight);
        writeU8(packet.data(), 7, redTailSignalLeft);
        writeU8(packet.data(), 8, whiteFrontLeftLed);
        writeU8(packet.data(), 9, redFrontLeftLed);
        writeU8(packet.data(), 10, whiteFrontRightLed);
        writeU8(packet.data(), 11, searchLightMarker);
        writeU8(packet.data(), 12, headLightRight);
        writeU8(packet.data(), 13, redFrontRightLed);
        writeU8(packet.data(), 14, headLightLeft);
        writeU8(packet.data(), 15, hazardLightLed);
        writeU8(packet.data(), 16, doorReleaseLeftLit);
        writeU8(packet.data(), 18, trainDoorControlLeft);

        writeU8(packet.data(), 19, doorInterlockLit); // front panel left
        writeU8(packet.data(), 20, snowBrakeLed);
        writeU8(packet.data(), 21, vcbLamp);
        writeU8(packet.data(), 22, lineVoltLamp);
        writeU8(packet.data(), 23, safetySystemIsolated);
        writeU8(packet.data(), 24, addLed);
        writeU8(packet.data(), 25, wheelSlipSlideLed);
        writeU8(packet.data(), 26, trainWashLed);

        writeU8(packet.data(), 29, displayYellowLed);

        writeU8(packet.data(), 31, awsLit);
        writeU8(packet.data(), 32, spadLit);
        writeU8(packet.data(), 33, overspeedLit);
        writeU8(packet.data(), 34, trainStopOverrideLit);
        writeU8(packet.data(), 35, tempIsolationFaultLit);
        writeU8(packet.data(), 36, frontPreparationLed);
        writeU8(packet.data(), 37, awsSunflowerLit);
        writeU8(packet.data(), 38, coupleLed);
        writeU8(packet.data(), 39, rearPreparationButton);
        writeU8(packet.data(), 40, draButtonLed);
        writeU8(packet.data(), 41, trainDoorControlRight);
        writeU8(packet.data(), 42, doorInterlockLit);
        writeU8(packet.data(), 43, doorReleaseRightLit); // front panel L

        writeU8(packet.data(), 44, electricLed);
        writeU8(packet.data(), 45, trainFaultAcknowledgeLed);

        writeU8(packet.data(), 46, demisterLed);
        writeU8(packet.data(), 47, stopDieselLed);
        writeU8(packet.data(), 48, engineStartLed);
        writeU8(packet.data(), 49, dieselLed);

        writeU8(packet.data(), 51, DoorInd2);
        writeU8(packet.data(), 52, CABHVACMain);

        writeU8(packet.data(), 54, sleepModeLed);
        writeU8(packet.data(), 55, lampOtmr);
        writeU8(packet.data(), 56, leftLocalDoorOnly);
        writeU8(packet.data(), 57, doorInterlockLeftBackPanelLit); // back panel left
        writeU8(packet.data(), 58, doorReleaseLeftBackPanelLit); // back panel left L
        writeU8(packet.data(), 59, doorReleaseLeftBackPanelLit); // back panel left R
        writeU8(packet.data(), 60, doorInterlockRightBackPanelLit);  // back panel left
        writeU8(packet.data(), 61, rightLocalDoorOnly);
        writeU8(packet.data(), 62, doorReleaseRightBackPanelLit); // back panel right L
        writeU8(packet.data(), 63, doorReleaseRightBackPanelLit); // back panel right R

        writeU8(packet.data(), 64, ATPControl);
        writeU8(packet.data(), 65, EmerBrakeValve);

        writeU8(packet.data(), 67, GSMR);
        writeU8(packet.data(), 68, ETCS);
        writeU8(packet.data(), 69, ChangeOverControl);
        writeU8(packet.data(), 70, EmerBrakeAlarm);
        writeU8(packet.data(), 71, ATPControl);
        writeU8(packet.data(), 72, VCBControl1);
        writeU8(packet.data(), 73, PantADD);
        writeU8(packet.data(), 74, PantControl);
        writeU8(packet.data(), 75, PantControlValve);
        writeU8(packet.data(), 76, DSD);
        writeU8(packet.data(), 77, SpeedSignalRelay);
        writeU8(packet.data(), 78, DoorControl);
        writeU8(packet.data(), 79, LocoHauledMode);
        writeU8(packet.data(), 80, Sanding);
        writeU8(packet.data(), 81, DoorControlPanel2);
        writeU8(packet.data(), 82, EnergyMeter);
        writeU8(packet.data(), 83, Wiper);
        writeU8(packet.data(), 84, BatLowVoltage);
        writeU8(packet.data(), 85, TCA);
        writeU8(packet.data(), 86, extLightPanel);
        writeU8(packet.data(), 87, RightExtLights);
        writeU8(packet.data(), 88, LeftExtLights);
        writeU8(packet.data(), 89, TMS2);
        writeU8(packet.data(), 90, TMS1);
        writeU8(packet.data(), 91, DeskLamps);
        writeU8(packet.data(), 92, FrontHatch);
        writeU8(packet.data(), 93, TMSMonitor);
        writeU8(packet.data(), 94, DSDPedal);
        writeU8(packet.data(), 95, CabCelinigLight);

        writeU8(packet.data(), 96, CabVent);
        writeU8(packet.data(), 97, EmerLightsControl);

        writeU8(packet.data(), 99, ExtStepLights);
        writeU8(packet.data(), 100, CabEmerLights);
        writeU8(packet.data(), 101, CoupleControl);
        writeU8(packet.data(), 102, CabDoorClose);
        writeU8(packet.data(), 103, CabSocketSub);
        writeU8(packet.data(), 104, CMD);
        writeU8(packet.data(), 105, CABHVACControl);
        writeU8(packet.data(), 106, ATPDisplay);
        writeU8(packet.data(), 107, TPWSAWS);
        writeU8(packet.data(), 108, APC);
        writeU8(packet.data(), 109, CPBCControl2);
        writeU8(packet.data(), 110, CPBCControl1);
        writeU8(packet.data(), 111, VCBControl2);
        writeU8(packet.data(), 112, BrakeControl5);
        writeU8(packet.data(), 113, CPBCControl3);
        writeU8(packet.data(), 114, BrakeSpeedSignal);
    }
    else if (driverDeskVersion == DriverDeskVersion::Lumo || driverDeskVersion == DriverDeskVersion::Hull)
    {
        writeU8(packet.data(), 6, searchLightMarker);
        writeU8(packet.data(), 7, redTailSignalLeft);
        writeU8(packet.data(), 8, whiteFrontLeftLed);
        writeU8(packet.data(), 9, whiteFrontRightLed);
        writeU8(packet.data(), 10, redTailSignalRight);

        writeU8(packet.data(), driverDeskVersion == DriverDeskVersion::Hull ? 35 : 11, headLightLeft);
        writeU8(packet.data(), 12, headLightRight);
        writeU8(packet.data(), 13, redFrontLeftLed);
        writeU8(packet.data(), 14, redFrontRightLed);
        writeU8(packet.data(), 15, hazardLightLed);
        writeU8(packet.data(), 16, doorReleaseLeftLit);
        writeU8(packet.data(), 17, doorReleaseLeftLit);
        writeU8(packet.data(), 18, trainDoorControlLeft);
        writeU8(packet.data(), 19, doorInterlockLit);
        writeU8(packet.data(), 20, lineVoltLamp);

        writeU8(packet.data(), 21, vcbLamp);
        writeU8(packet.data(), 22, safetySystemIsolated);
        writeU8(packet.data(), 23, addLed);
        writeU8(packet.data(), 24, snowBrakeLed);
        writeU8(packet.data(), 25, spadLit);
        writeU8(packet.data(), 26, overspeedLit);
        writeU8(packet.data(), 27, tempIsolationFaultLit);
        writeU8(packet.data(), 28, trainStopOverrideLit);
        writeU8(packet.data(), 29, awsLit);
        writeU8(packet.data(), driverDeskVersion == DriverDeskVersion::Lumo ? 35: 30, draButtonLed);

        writeU8(packet.data(), 31, awsSunflowerLit);
        writeU8(packet.data(), 32, frontPreparationLed);
        writeU8(packet.data(), 33, coupleLed);
        writeU8(packet.data(), 34, rearPreparationButton);
        writeU8(packet.data(), 36, wheelSlipSlideLed);
        writeU8(packet.data(), 37, trainWashLed);
        writeU8(packet.data(), 38, doorReleaseRightLit);
        writeU8(packet.data(), 39, doorReleaseRightLit);
        writeU8(packet.data(), 40, doorInterlockLit);

        writeU8(packet.data(), 42, trainFaultAcknowledgeLed);
        writeU8(packet.data(), 41, trainDoorControlRight);
        writeU8(packet.data(), 43, electricLed);
        writeU8(packet.data(), 44, demisterLed);
        writeU8(packet.data(), 45, engineStartLed);
        writeU8(packet.data(), 46, dieselLed);
        writeU8(packet.data(), 47, stopDieselLed);
        writeU8(packet.data(), driverDeskVersion == DriverDeskVersion::Lumo ? 50 : 49, displayYellowLed);

        writeU8(packet.data(), 56, PantControl);
        writeU8(packet.data(), 57, PantControlValve);
        writeU8(packet.data(), 58, PantADD);
        writeU8(packet.data(), 59, VCBControl1);
        writeU8(packet.data(), 60, VCBControl2);

        writeU8(packet.data(), 61, APC);
        writeU8(packet.data(), 62, CPBCControl1);
        writeU8(packet.data(), 63, CPBCControl2);
        writeU8(packet.data(), 64, CPBCControl3);
        writeU8(packet.data(), 65, BrakeControl5);
        writeU8(packet.data(), 66, BrakeSpeedSignal);
        writeU8(packet.data(), 67, CABHVACMain);
        writeU8(packet.data(), 68, ETCS);
        writeU8(packet.data(), 69, ATPControl);

        writeU8(packet.data(), 73, TPWSAWS);
        writeU8(packet.data(), 74, GSMR);
        writeU8(packet.data(), 75, DSD);
        writeU8(packet.data(), 76, EmerBrakeValve);
        writeU8(packet.data(), 77, EmerBrakeAlarm);
        writeU8(packet.data(), 78, ChangeOverControl);
        writeU8(packet.data(), 79, DoorControl);
        writeU8(packet.data(), 80, DoorControlPanel2);

        writeU8(packet.data(), 81, DoorInd2);
        writeU8(packet.data(), 82, Sanding);
        writeU8(packet.data(), 84, LocoHauledMode);
        writeU8(packet.data(), 85, BatLowVoltage);
        writeU8(packet.data(), 86, CoupleControl);
        writeU8(packet.data(), 87, FrontHatch);
        writeU8(packet.data(), 88, DeskLamps);
        writeU8(packet.data(), 89, TMS2);
        writeU8(packet.data(), 90, TMS1);

        writeU8(packet.data(), 91, TMSMonitor);
        writeU8(packet.data(), 92, LeftExtLights);
        writeU8(packet.data(), 93, RightExtLights);
        writeU8(packet.data(), 94, extLightPanel);
        writeU8(packet.data(), 95, Wiper);
        writeU8(packet.data(), 96, TCA);
        writeU8(packet.data(), 97, EnergyMeter);
        writeU8(packet.data(), 98, CabSocketSub);
        writeU8(packet.data(), 99, CABHVACControl);
        writeU8(packet.data(), 100, CabVent);

        writeU8(packet.data(), 101, ExtStepLights);
        writeU8(packet.data(), 102, CabEmerLights);
        writeU8(packet.data(), 103, CabDoorClose);
        writeU8(packet.data(), 104, CabCelinigLight);
        writeU8(packet.data(), 105, EmerLightsControl);
        writeU8(packet.data(), 106, DSDPedal);
        writeU8(packet.data(), 107, CMD);
        writeU8(packet.data(), 108, SpeedSignalRelay);
        writeU8(packet.data(), 109, eCouplerIsolation);
        writeU8(packet.data(), 110, sleepModeLed);
        writeU8(packet.data(), 111, lampOtmr);
        writeU8(packet.data(), 112, leftLocalDoorOnly);
        writeU8(packet.data(), 113, doorInterlockLeftBackPanelLit);
        writeU8(packet.data(), 114, doorReleaseLeftBackPanelLit);
        writeU8(packet.data(), 115, doorReleaseLeftBackPanelLit);
    }
    else if (driverDeskVersion == DriverDeskVersion::Emr)
    {
        auto const* const mapping = reinterpret_cast<packet_io::Packet::value_type const*>(this);
        packet.assign(mapping, mapping + sizeof(VarsToControlPanel));
    }
    return packet;
}

void VarsToControlPanel::deserialize(const packet_io::Packet& packet)
{
    auto* const dest = reinterpret_cast<packet_io::Packet::value_type*>(this);
    auto const* const source = packet.data();

    std::copy(source, source + std::min(sizeof(VarsToControlPanel), packet.size()), dest);
}

} // namespace Hitachi
