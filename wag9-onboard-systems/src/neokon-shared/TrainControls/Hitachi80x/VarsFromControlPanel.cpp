#include "neokon-shared/TrainControls/Hitachi80x/VarsFromControlPanel.h"
#include "neokon-shared/TrainControls/ControlPanelSerialization.h"

namespace Hitachi
{
void
VarsFromControlPanel::deserialize(uint8_t const* data, size_t size)
{
    using namespace packet_io;
    size_t const packetSize = driverDeskVersion == DriverDeskVersion::Euston ? 170 :
        driverDeskVersion == DriverDeskVersion::Crewe ? 168 : 157;
    if (size < packetSize)
    {
        return;
    }

    if (driverDeskVersion != DriverDeskVersion::Emr &&
        driverDeskVersion != DriverDeskVersion::Lumo && driverDeskVersion != DriverDeskVersion::Hull)
    {
        bufferLights = parseU8(data, 1);
        if (driverDeskVersion == DriverDeskVersion::Euston)
        {
            if (bufferLights == 6)
            {
                bufferLights = 1;
            }
            else
            {
                bufferLights += 1;
            }
        }

        wipers = parseU8(data, 6);
        throttleController = parseU8(data, 7);

        if (driverDeskVersion == DriverDeskVersion::Crewe)
        {
            int newDds = parseU8(data, 8);

            dds = newDds < 25 ? DriverDirectionSwitch::Off :
                newDds < 50 ? DriverDirectionSwitch::Forward :
                newDds < 90 ? DriverDirectionSwitch::Neutral : DriverDirectionSwitch::Reverse;
            masterKeySwitch = parseU8(data, 9);
        }
        else if (driverDeskVersion == DriverDeskVersion::Euston)
        {
            int newDds = parseU8(data, 9);
            dds = newDds < 15 ? DriverDirectionSwitch::Off :
                newDds < 70 ? DriverDirectionSwitch::Forward :
                newDds < 140 ? DriverDirectionSwitch::Neutral : DriverDirectionSwitch::Reverse;

            masterKeySwitch = parseU8(data, 8);
        }

        wipersSpeedHz = parseU8(data, 10);

        if (driverDeskVersion == DriverDeskVersion::Crewe)
        {
            pedal = parseU8(data, 11);
            footPedal = parseU8(data, 15);
        }
        else if (driverDeskVersion == DriverDeskVersion::Euston)
        {
            pedal = parseU8(data, 15);
            footPedal = parseU8(data, 11);
        }

        rightEmergencyButton = parseU8(data, 12);
        typhon = parseU8(data, 13);
        whistle = parseU8(data, 14);


        hazardLight = parseU8(data, 20);

        frontPreparationButton = parseU8(data, 25);
        rearPreparationButton = parseU8(data, 26);
        coupleButton = parseU8(data, 27);
        uncoupleButton = parseU8(data, 28);
        slipButton = parseU8(data, 29);
        trainWashButton = parseU8(data, 30);

        awsResetPressed = parseU8(data, 32);
        draButton = parseU8(data, 33);
        signalFrontPanelRight = parseU8(data, 34);
        doorReleaseFrontRightR = parseU8(data, 35);
        doorReleaseFrontRightL = parseU8(data, 36);
        doorCloseInterlockFrontRight = parseU8(data, 37);

        dsdPanelButton = parseU8(data, 39);
        trainFaultAcknowledge = parseU8(data, 40);
        electricButton = parseU8(data, 41);
        panUpButton = parseU8(data, 42);
        panDownButton = parseU8(data, 43);
        washerEnabled = parseU8(data, 44);
        windscreenDemist = parseU8(data, 45);
        signalFrontPanelLeft = parseU8(data, 46);
        doorReleaseFrontLeftL = parseU8(data, 47);
        doorReleaseFrontLeftR = parseU8(data, 48);

        doorCloseInterlockFrontLeft = parseU8(data, 50);
        buttonCCTV = parseU8(data, 51);

        leftEmergencyButton = parseU8(data, 52);
        snowBrakeButton = parseU8(data, 53);
        spadPressed = parseU8(data, 54);
        overspeedPressed = parseU8(data, 55);
        awsPressed = parseU8(data, 56);
        trainStopOverride = parseU8(data, 57);
        brakeReleasePressed = parseU8(data, 58);
        engineStartButton = parseU8(data, 59);
        dieselButton = parseU8(data, 60);
        engineStopButton = parseU8(data, 61);
        emergencyDieselStop = parseU8(data, 62);

        sleepModeDeactivation = parseU8(data, 70);
        auxOffButton = parseU8(data, 71);
        tmsReset = parseU8(data, 72);

        unitHatchClose = parseU8(data, 74);
        if (driverDeskVersion == DriverDeskVersion::Crewe)
        {
            auxOnButton = parseU8(data, 75);
            unattended = !parseU8(data, 77);
        }
        else if (driverDeskVersion == DriverDeskVersion::Euston)
        {
            auxOnButton = parseU8(data, 76);
            unattended = !parseU8(data, 78);
        }

        tpwsTemporaryIsolation = TpwsTemporaryIsolation::None;
        if (driverDeskVersion == DriverDeskVersion::Crewe)
        {
            if (parseU8(data, 83))
                tpwsTemporaryIsolation = TpwsTemporaryIsolation::Isolated;
            if (parseU8(data, 84))
                tpwsTemporaryIsolation = TpwsTemporaryIsolation::Normal;
        }
        else if (driverDeskVersion == DriverDeskVersion::Euston)
        {
            if (parseU8(data, 84))
                tpwsTemporaryIsolation = TpwsTemporaryIsolation::Isolated;
            if (parseU8(data, 83))
                tpwsTemporaryIsolation = TpwsTemporaryIsolation::Normal;
        }

        buttonCPBC = parseU8(data, 90);

        if (driverDeskVersion == DriverDeskVersion::Crewe)
        {
            airConController = parseU8(data, 2);

            leftDoorControl = parseU8(data, 49);
            rightDoorControl = parseU8(data, 38);

            trainCommunicationPA2 = parseU8(data, 64);
            switchEtcsIsolation = parseU8(data, 85);
            switchTractionIntrlockIsolation = parseU8(data, 86);
            switchDraIsolation = parseU8(data, 87);
            switchSdoIsolation = parseU8(data, 88);
            switchAwsIsolation = parseU8(data, 89);
            switchVigilanceIsolation = parseU8(data, 91);
            switchDsdIsolation = parseU8(data, 92);
            switchBrAtpIsolation = parseU8(data, 93);
            doorCloseInterlockBackLeft = parseU8(data, 94);
            doorReleaseBackLeftL = parseU8(data, 95);
            doorReleaseBackLeftR = parseU8(data, 96);
            signalBackPanelLeft = parseU8(data, 97);
            buzzerOnlyOffLeft = parseU8(data, 98);

            doorCloseInterlockBackRight = parseU8(data, 99);
            doorReleaseBackRightL = parseU8(data, 100);
            doorReleaseBackRightR = parseU8(data, 101);
            signalBackPanelRight = parseU8(data, 102);
            buzzerOnlyOffRight = parseU8(data, 103);
            buzzerOnlyOnRight = parseU8(data, 104);
            buzzerOnlyOnLeft = parseU8(data, 105);

            tmsMonitor = parseU8(data, 110);
            tms1 = parseU8(data, 111);
            tms2 = parseU8(data, 112);

            mcbFrontHatch = parseU8(data, 114);
            coupleControlMcb = parseU8(data, 115);

            sandingMcb = parseU8(data, 119);

            cbDsdPedal = parseU8(data, 125);

            brakeControlMcb = parseU8(data, 135);
            cpbc3Mcb = parseU8(data, 136);
            cpbc2Mcb = parseU8(data, 137);
            cpbc1Mcb = parseU8(data, 138);

            vcb2Mcb = parseU8(data, 140);
            vcb1Mcb = parseU8(data, 141);
            pantAdd = parseU8(data, 142);
            pantControlValve = parseU8(data, 143);
            pantControl = parseU8(data, 144);

            //Useless
            apcMcb = parseU8(data, 139);
            brakeSpeedSignalMcb = parseU8(data, 134);
            cabHyacMain = parseU8(data, 154);
            atpDisplayMcb = parseU8(data, 133);
            changeoverControlMcb = parseU8(data, 145);
            emerBrakeAlarmMcb = parseU8(data, 146);
            emerBrakeValveMcb = parseU8(data, 147);
            doorControlMcb = parseU8(data, 123);
            doorControlPanelMcb = parseU8(data, 121);
            doorIndication = parseU8(data, 120);
            speedSignalRelayMcb = parseU8(data, 118);
            locoHauledMode = parseU8(data, 117);
            batLowVoltage = parseU8(data, 116);
            deskLapmsMcb = parseU8(data, 113);
            tcaMcb = parseU8(data, 156);
            energyMeterMcb = parseU8(data, 155);
            extLightPanelMcb = parseU8(data, 158);
            rightExtLightsMcb = parseU8(data, 108);
            leftExtLightsMcb = parseU8(data, 109);
            cmdMcb = parseU8(data, 124);
            cabSocket = parseU8(data, 162);
            cabSocketSub = parseU8(data, 163);
            cabHvacControlMcb = parseU8(data, 132);
            cabVentMcb = parseU8(data, 131);
            extStepLightsMcb = parseU8(data, 130);
            cabEmerLightsMcb = parseU8(data, 129);
            cabCeilingLightMcb = parseU8(data, 127);
            emerLightControlMcb = parseU8(data, 126);
            cabDoorCloseMcb = parseU8(data, 128);

            cbDsd = parseU8(data, 148);
            gsmrMcb = parseU8(data, 149);
            cbTpwsAws = parseU8(data, 150);

            etcsMcb = parseU8(data, 153);

            wipersMcb = parseU8(data, 157);

            switchTpwsAwsIsolation = parseU8(data, 164);
            switchPassCommIsolation = parseU8(data, 165);
            switchAddIsolation = parseU8(data, 166);
        }
        else if (driverDeskVersion == DriverDeskVersion::Euston)
        {
            switchEtcsIsolation = parseU8(data, 96);
            switchTractionIntrlockIsolation = parseU8(data, 97);
            switchDraIsolation = parseU8(data, 98);
            switchSdoIsolation = parseU8(data, 99);
            switchAwsIsolation = parseU8(data, 100);
            switchVigilanceIsolation = parseU8(data, 105);
            switchDsdIsolation = parseU8(data, 106);
            switchBrAtpIsolation = parseU8(data, 107);
            doorCloseInterlockBackLeft = parseU8(data, 85);
            doorReleaseBackLeftL = parseU8(data, 86);
            doorReleaseBackLeftR = parseU8(data, 87);
            signalBackPanelLeft = parseU8(data, 88);
            buzzerOnlyOffLeft = parseU8(data, 169);
            trainCommunicationPA2 = parseU8(data, 64);

            doorCloseInterlockBackRight = parseU8(data, 91);
            doorReleaseBackRightL = parseU8(data, 92);
            doorReleaseBackRightR = parseU8(data, 93);
            signalBackPanelRight = parseU8(data, 94);
            buzzerOnlyOffRight = parseU8(data, 161);
            buzzerOnlyOnRight = parseU8(data, 95);
            buzzerOnlyOnLeft = parseU8(data, 89);

            tmsMonitor = parseU8(data, 111);
            tms1 = parseU8(data, 112);
            tms2 = parseU8(data, 113);

            mcbFrontHatch = parseU8(data, 115);

            sandingMcb = parseU8(data, 121);

            cbDsdPedal = parseU8(data, 126);

            brakeControlMcb = parseU8(data, 135);
            cpbc3Mcb = parseU8(data, 136);
            cpbc2Mcb = parseU8(data, 137);
            cpbc1Mcb = parseU8(data, 138);

            vcb2Mcb = parseU8(data, 140);
            vcb1Mcb = parseU8(data, 141);
            pantAdd = parseU8(data, 142);
            pantControlValve = parseU8(data, 143);
            pantControl = parseU8(data, 144);

            // TODO: сделать нормальное чтение строки в Юстон
            coupleControlMcb = true;

            //Useless
            apcMcb = true;
            brakeSpeedSignalMcb = true;
            cabHyacMain = true;
            atpDisplayMcb = true;
            changeoverControlMcb = true;
            emerBrakeAlarmMcb = true;
            emerBrakeValveMcb = true;
            doorControlMcb = true;
            doorControlPanelMcb = true;
            doorIndication = true;
            speedSignalRelayMcb = true;
            locoHauledMode = true;
            batLowVoltage = true;
            deskLapmsMcb = true;
            tcaMcb = true;
            energyMeterMcb = true;
            extLightPanelMcb = true;
            rightExtLightsMcb = true;
            leftExtLightsMcb = true;
            cmdMcb = true;
            cabSocket = true;
            cabSocketSub = true;
            cabHvacControlMcb = true;
            cabVentMcb = true;
            extStepLightsMcb = true;
            cabEmerLightsMcb = true;
            cabCeilingLightMcb = true;
            emerLightControlMcb = true;
            cabDoorCloseMcb = true;

            cbDsd = parseU8(data, 154);
            gsmrMcb = parseU8(data, 148);
            cbTpwsAws = parseU8(data, 149);

            etcsMcb = parseU8(data, 153);

            wipersMcb = parseU8(data, 108);

            switchTpwsAwsIsolation = parseU8(data, 104);
            switchPassCommIsolation = parseU8(data, 102);
            switchAddIsolation = parseU8(data, 103);

            switchEmergencyBrakeSuplyIsolation = parseU8(data, 101);
        }
    }
    else if (driverDeskVersion == DriverDeskVersion::Lumo || driverDeskVersion == DriverDeskVersion::Hull)
    {
        buttonCPBC = parseU8(data, 0);
        bufferLights = parseU8(data, 1) == 2 ? 6 : parseU8(data, 1) - 2;
        if (driverDeskVersion == DriverDeskVersion::Hull)
        {
            bufferLights -= 1;

            if (parseU8(data, 1) == 1) bufferLights = 4;
            if (parseU8(data, 1) == 2) bufferLights = 5;
            if (parseU8(data, 1) == 3) bufferLights = 6;
        }

        airConController = parseU8(data, 2);
        heather = parseU8(data, 3);
        docsLights = parseU8(data, 4);
        deskLights = parseU8(data, 5);
        wipers = parseU8(data, 6);
        {
            int newDds = parseU8(data, 7);

            if (driverDeskVersion == DriverDeskVersion::Lumo)
            {
                dds = newDds < 25 ? DriverDirectionSwitch::Off :
                    newDds < 50 ? DriverDirectionSwitch::Forward :
                    newDds < 90 ? DriverDirectionSwitch::Neutral : DriverDirectionSwitch::Reverse;
            }
            else
            {
                dds = newDds < 70 ? DriverDirectionSwitch::Off :
                    newDds < 115 ? DriverDirectionSwitch::Forward :
                    newDds < 150 ? DriverDirectionSwitch::Neutral : DriverDirectionSwitch::Reverse;
            }
        }
        masterKeySwitch = parseU8(data, 8);
        throttleController = parseU8(data, 9);
        wipersSpeedHz = parseU8(data, 10);
        tpwsTemporaryIsolation = TpwsTemporaryIsolation::None;
        if (driverDeskVersion == DriverDeskVersion::Lumo)
        {
            if (parseU8(data, 155))
                tpwsTemporaryIsolation = TpwsTemporaryIsolation::Isolated;
            if (parseU8(data, 97))
                tpwsTemporaryIsolation = TpwsTemporaryIsolation::Normal;
        }
        footPedal = parseU8(data, 11);
        rightEmergencyButton = parseU8(data, 12);
        typhon = parseU8(data, 14);
        whistle = parseU8(data, 13);
        pedal = parseU8(data, 15);

        hazardLight = parseU8(data, 20);
        dsdPeadlUp = parseU8(data, 21);
        dsdPeadlDown = parseU8(data, 22);
        downLighter = parseU8(data, 23);
        cabCentralLight = parseU8(data, 24);
        signalFrontPanelLeft = parseU8(data, 25);
        doorReleaseFrontLeftL = parseU8(data, 26);
        doorReleaseFrontLeftR = parseU8(data, 27);

        doorCloseInterlockFrontLeft = parseU8(data, 29);
        cctvButton = parseU8(data, 30);

        leftEmergencyButton = parseU8(data, 31);
        snowBrakeButton = parseU8(data, 32);
        spadPressed = parseU8(data, 33);
        overspeedPressed = parseU8(data, 34);
        trainStopOverride = parseU8(data, 35);
        brakeReleasePressed = parseU8(data, 36);
        awsPressed = parseU8(data, 37);
        draButton = parseU8(data, 38);
        frontPreparationButton = parseU8(data, 39);
        coupleButton = parseU8(data, 40);

        rearPreparationButton = parseU8(data, 41);
        uncoupleButton = parseU8(data, 42);
        wheelSlipSlide = parseU8(data, 43);
        trainWashButton = parseU8(data, 44);
        awsResetPressed = parseU8(data, 45);
        signalFrontPanelRight = parseU8(data, 46);
        doorReleaseFrontRightL = parseU8(data, 47);
        doorReleaseFrontRightR = parseU8(data, 48);
        dsdPanelButton = parseU8(data, 49);
        trainFaultAcknowledge = parseU8(data, 50);

        electricButton = parseU8(data, 51);
        panUpButton = parseU8(data, 52);
        washerEnabled = parseU8(data, 53);
        windscreenDemist = parseU8(data, 54);
        engineStartButton = parseU8(data, 55);
        dieselButton = parseU8(data, 56);
        panDownButton = parseU8(data, 57);
        doorCloseInterlockFrontRight =
            parseU8(data, driverDeskVersion == DriverDeskVersion::Lumo ? 59 : 58);

        engineStopButton = parseU8(data, 60);
        emergencyDieselStop = parseU8(data, 61);
        trainCommunicationPA2 = parseU8(data, 70);

        etcsIso = parseU8(data, 72);
        switchEtcsIsolation = parseU8(data, 72);
        switchAwsIsolation = parseU8(data, 73);
        switchTpwsAwsIsolation = parseU8(data, 74);
        tractionIso = parseU8(data, 75);
        switchTractionIntrlockIsolation = parseU8(data, 75);
        emergencyBrakeSupplyIso = parseU8(data, 76);
        switchEmergencyBrakeSuplyIsolation = parseU8(data, 76);
        switchVigilanceIsolation = parseU8(data, driverDeskVersion == DriverDeskVersion::Hull ? 80 : 77);
        switchDraIsolation = parseU8(data, 78);
        passComIso = parseU8(data, 79);
        switchPassCommIsolation = parseU8(data, 79);
        switchDsdIsolation = parseU8(data, driverDeskVersion == DriverDeskVersion::Hull ? 77 : 80);

        switchSdoIsolation = parseU8(data, 81);
        addIsolation = parseU8(data, 82);
        switchAddIsolation = parseU8(data, 82);
        brAtpIso = parseU8(data, 83);
        eCouplerIsolation = parseU8(data, 84);
        unitHatchClose = parseU8(data, 85);
        pantographChanger = parseU8(data, 86);
        sleepModeDeactivation = parseU8(data, 87);
        rescueMode = parseU8(data, 88);
        auxOffButton = parseU8(data, 90);

        auxOnButton = parseU8(data, 91);
        tmsReset = parseU8(data, 92);
        locoHauledMode = parseU8(data, 93);
        switcherEtcsBr = parseU8(data, 94);
        emergencyLightsOff = parseU8(data, 95);
        unattended = !parseU8(data, 96);
        doorCloseInterlockBackLeft = parseU8(data, 98);
        doorReleaseBackLeftL = parseU8(data, 99);
        doorReleaseBackLeftR = parseU8(data, 100);

        buzzerOnlyOnLeft = parseU8(data, 101);
        signalBackPanelLeft = parseU8(data, 102);
        pantControl = parseU8(data, 103);
        pantControlValve = parseU8(data, 104);
        pantAdd = parseU8(data, 105);
        vcb1Mcb = parseU8(data, 106);
        vcb2Mcb = parseU8(data, 107);
        apcMcb = parseU8(data, 108);
        cpbc1Mcb = parseU8(data, 109);
        cpbc2Mcb = parseU8(data, 110);

        cpbc3Mcb = parseU8(data, 111);
        brakeControlMcb = parseU8(data, 112);
        brakeSpeedSignalMcb = parseU8(data, 113);
        cabHyacMain = parseU8(data, 114);
        etcsMcb = parseU8(data, 115);
        cctvMcb = parseU8(data, 116);
        cbTpwsAws = parseU8(data, 119);
        gsmrMcb = parseU8(data, 120);

        cbDsd = parseU8(data, 121);
        emerBrakeValveMcb = parseU8(data, 122);
        emerBrakeAlarmMcb = parseU8(data, 123);
        changeoverControlMcb = parseU8(data, 124);
        doorControlMcb = parseU8(data, 125);
        doorControlPanelMcb = parseU8(data, 126);
        doorIndication = parseU8(data, 127);
        sandingMcb = parseU8(data, 128);
        speedSignalRelayMcb = parseU8(data, 129);
        locoHauledMode = parseU8(data, 130);

        batLowVoltage = parseU8(data, 131);
        coupleControlMcb = parseU8(data, 132);
        mcbFrontHatch = parseU8(data, 133);
        deskLapmsMcb = parseU8(data, 134);
        tms1 = parseU8(data, 135);
        tms2 = parseU8(data, 136);
        tmsMonitor = parseU8(data, 137);
        leftExtLightsMcb = parseU8(data, 138);
        rightExtLightsMcb = parseU8(data, 139);
        extLightPanelMcb = parseU8(data, 140);

        wipersMcb = parseU8(data, 141);
        tcaMcb = parseU8(data, 142);
        energyMeterMcb = parseU8(data, 143);
        cabSocket = parseU8(data, 144);
        cabHvacControlMcb = parseU8(data, 145);
        cabVentMcb = parseU8(data, 146);
        extStepLightsMcb = parseU8(data, 147);
        cabEmerLightsMcb = parseU8(data, 148);
        cabDoorCloseMcb = parseU8(data, 149);
        cabCeilingLightMcb = parseU8(data, 150);
        emerLightControlMcb = parseU8(data, 151);
        cbDsdPedal = parseU8(data, 152);
        cmdMcb = parseU8(data, 153);
        cabSocketSub = parseU8(data, 154);
        buzzerOnlyOffLeft = parseU8(data, 156);
        leftDoorControl = parseU8(data, 28);
        rightDoorControl = parseU8(data, driverDeskVersion == DriverDeskVersion::Lumo ? 58 : 59);

        tpwsTemporaryIsolation = TpwsTemporaryIsolation::None;

        if (parseU8(data, 155))
            tpwsTemporaryIsolation = TpwsTemporaryIsolation::Isolated;
        if (parseU8(data, 97))
            tpwsTemporaryIsolation = TpwsTemporaryIsolation::Normal;
    }
    else if (driverDeskVersion == DriverDeskVersion::Emr)
    {
        auto* const mapping = reinterpret_cast<Packet::value_type*>(this);
        std::copy(data, data + std::min(size, sizeof(VarsFromControlPanel)), mapping);
    }
}
} //namespace Hitachi
