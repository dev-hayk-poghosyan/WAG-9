#include "DynamicElectrificationCatenaryGenerator.h"

#include "DynamicElectrification.h"

#include "neokon-shared/BezierSegmentHelpers.h"
#include "neokon-shared/electrification/CatenaryGenerationConstants.h"
#include "neokon-shared/electrification/DynamicElectrification.h"
#include "neokon-shared/electrification/DynamicElectrificationCatenaryWire.h"
#include "neokon-shared/electrification/DynamicElectrificationCatenaryAttachable.h"
#include "neokon-shared/FloatingPointCompare.h"
#include "neokon-shared/GenerateMeshFromSplineList.h"
#include "neokon-shared/TransformByTwoPoints.h"

#include <algorithm>
#include <random>

using namespace electrification;

const double ConnectorOffset = 0.15;

double
maxCableOffsetByCoord(Math::Vec3 const& startPos, Math::Vec3 const& endPos,
    double saggingCoefficient, bool isStitched)
{
    //аппроксимация по нескольким заданным парам {длина кабеля, длина минимальной струны}
    auto distanceByLength = [&isStitched, &saggingCoefficient](double length)
    {
        double distance { 0.0 };
        if (isStitched)
        {
            distance = 0.000221866 * length * length - 0.000677761 * length +
                0.0232648 * saggingCoefficient;
        }
        else
        {
            distance =
                0.000227273 * length * length - 0.00877273 * length + saggingCoefficient * 0.0862727;
        }

        return distance;
    };

    double cableLength = (endPos - startPos).length();
    double offset = distanceByLength(cableLength);
    return offset;
}

/////////////////////////////////////////////////////////////////////////////////////
// class DynamicElectrificationCatenaryGenerator
/////////////////////////////////////////////////////////////////////////////////////
DynamicElectrificationCatenaryGenerator::ConnectTwoPolesByCatenaryArgs::ConnectTwoPolesByCatenaryArgs(
    electrification::PoleContextData const& leftPole,
    electrification::PoleContextData const& rightPole,
    ObjectMessageLogger* log)
    : leftPole(leftPole)
    , rightPole(rightPole)
    , log(log)
{
}

//static
std::list<DynamicElectrificationCatenaryGenerator::CreateArgs>
DynamicElectrificationCatenaryGenerator::connectTwoPolesByCatenary(
    ConnectTwoPolesByCatenaryArgs const& args)
{
    using namespace electrification;

    std::list<DynamicElectrificationCatenaryGenerator::CreateArgs> result;

    auto generatedHost = DynamicElectrification::instance();
    assert(generatedHost);

    auto leftPoleChooser
        = generatedHost->getOrCreateNodeNameChooser(args.leftPolePreset);
    if (!leftPoleChooser)
    {
        if (args.log)
        {
            args.log->error("Can not load preset: %s\n", args.leftPolePreset.c_str());
        }
        return result;
    }

    auto rightPoleChooser = generatedHost->getOrCreateNodeNameChooser(args.rightPolePreset);
    if (!rightPoleChooser)
    {
        if (args.log)
        {
            args.log->error("Can not load preset: %s\n", args.rightPolePreset.c_str());
        }
        return result;
    }

    std::optional<std::string> leftPoleNodeName = args.leftPole.isSuspension ?
        leftPoleChooser->choosePortalSuspensionNodeName(args.leftPole)
        : leftPoleChooser->choosePoleNodeName(args.leftPole);
    if (!leftPoleNodeName)
    {
        if (args.log)
        {
            args.log->error(
                "Can not choose pole node name satisfying the requirements of the preset: %s\n",
                args.leftPolePreset.c_str());
        }
        return result;
    }

    std::optional<std::string> rightPoleNodeName = args.rightPole.isSuspension ?
        rightPoleChooser->choosePortalSuspensionNodeName(args.rightPole)
        : rightPoleChooser->choosePoleNodeName(args.rightPole);
    if (!rightPoleNodeName)
    {
        if (args.log)
        {
            args.log->error(
                "Can not choose pole node name satisfying the requirements of the preset: %s\n",
                args.rightPolePreset.c_str());
        }
        return result;
    }

    std::optional<PoleSourceNodeHelperData> leftPoleHelperData
        = findPoleNodeHelperData(*leftPoleNodeName);
    std::optional<PoleSourceNodeHelperData> rightPoleHelperData
        = findPoleNodeHelperData(*rightPoleNodeName);
    if (!leftPoleHelperData || !rightPoleHelperData)
    {
        if (args.log)
        {
            args.log->error("Can not load connector transforms for pole nodes: 1: %s, 2: %s\n",
                leftPoleNodeName->c_str(), rightPoleNodeName->c_str());
        }
        return result;
    }

    if (leftPoleHelperData->connectorsByNames.size() < 2)
    {
        if (args.log)
        {
            args.log->error(
                "Some connectors not found for pole node %s \n", leftPoleNodeName->c_str());
        }
        return result;
    }

    if (rightPoleHelperData->connectorsByNames.size() < 2)
    {
        if (args.log)
        {
            args.log->error(
                "Some connectors not found for pole node %s \n", rightPoleNodeName->c_str());
        }
        return result;
    }

    const std::list<std::pair<std::string, std::string>> allowedTransionalWirePairs
    {
        std::make_pair(POLE_CONNECTOR_ID_NT_WS_L, POLE_CONNECTOR_ID_NT_WS_L),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS_L, POLE_CONNECTOR_ID_NT_WS),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS_R, POLE_CONNECTOR_ID_NT_WS_R),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS_R, POLE_CONNECTOR_ID_NT_WA),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS_R, POLE_CONNECTOR_ID_NT_WN),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS, POLE_CONNECTOR_ID_NT_WS_R),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS, POLE_CONNECTOR_ID_NT_WA),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS, POLE_CONNECTOR_ID_NT_WN),
        std::make_pair(POLE_CONNECTOR_ID_NT_WA, POLE_CONNECTOR_ID_NT_WS),
        std::make_pair(POLE_CONNECTOR_ID_NT_WA, POLE_CONNECTOR_ID_NT_WS_L),
        std::make_pair(POLE_CONNECTOR_ID_NT_WN, POLE_CONNECTOR_ID_NT_WS),
        std::make_pair(POLE_CONNECTOR_ID_NT_WN, POLE_CONNECTOR_ID_NT_WS_L)
    };

    const std::list<std::pair<std::string, std::string>> allowedWirePairsCommon{
        std::make_pair(POLE_CONNECTOR_ID_NT_WS_L, POLE_CONNECTOR_ID_NT_WS),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS_L, POLE_CONNECTOR_ID_NT_WS_L),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS_R, POLE_CONNECTOR_ID_NT_WS),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS_R, POLE_CONNECTOR_ID_NT_WS_R),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS, POLE_CONNECTOR_ID_NT_WS),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS_L, POLE_CONNECTOR_ID_NT_WA),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS_R, POLE_CONNECTOR_ID_NT_WA),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS, POLE_CONNECTOR_ID_NT_WA),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS, POLE_CONNECTOR_ID_NT_WN)
    };

    const std::list<std::pair<std::string, std::string>> allowedWirePairsPP{
        std::make_pair(POLE_CONNECTOR_ID_NT_WS_L, POLE_CONNECTOR_ID_NT_WS),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS_L, POLE_CONNECTOR_ID_NT_WS_L),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS, POLE_CONNECTOR_ID_NT_WS_L),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS, POLE_CONNECTOR_ID_NT_WS),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS_L, POLE_CONNECTOR_ID_NT_WA),
        std::make_pair(POLE_CONNECTOR_ID_NT_WA, POLE_CONNECTOR_ID_NT_WS_L),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS, POLE_CONNECTOR_ID_NT_WA),
        std::make_pair(POLE_CONNECTOR_ID_NT_WA, POLE_CONNECTOR_ID_NT_WS),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS, POLE_CONNECTOR_ID_NT_WN)
    };

    const std::list<std::pair<std::string, std::string>> allowedWirePairsPO{
        std::make_pair(POLE_CONNECTOR_ID_NT_WS_L, POLE_CONNECTOR_ID_NT_WS),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS_L, POLE_CONNECTOR_ID_NT_WS_R),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS, POLE_CONNECTOR_ID_NT_WS_R),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS, POLE_CONNECTOR_ID_NT_WS),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS_L, POLE_CONNECTOR_ID_NT_WA),
        std::make_pair(POLE_CONNECTOR_ID_NT_WA, POLE_CONNECTOR_ID_NT_WS_R),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS, POLE_CONNECTOR_ID_NT_WA),
        std::make_pair(POLE_CONNECTOR_ID_NT_WA, POLE_CONNECTOR_ID_NT_WS),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS, POLE_CONNECTOR_ID_NT_WN)
    };

    const std::list<std::pair<std::string, std::string>> allowedWirePairsOP{
        std::make_pair(POLE_CONNECTOR_ID_NT_WS_R, POLE_CONNECTOR_ID_NT_WS),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS_R, POLE_CONNECTOR_ID_NT_WS_L),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS, POLE_CONNECTOR_ID_NT_WS_L),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS, POLE_CONNECTOR_ID_NT_WS),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS_R, POLE_CONNECTOR_ID_NT_WA),
        std::make_pair(POLE_CONNECTOR_ID_NT_WA, POLE_CONNECTOR_ID_NT_WS_L),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS, POLE_CONNECTOR_ID_NT_WA),
        std::make_pair(POLE_CONNECTOR_ID_NT_WA, POLE_CONNECTOR_ID_NT_WS),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS, POLE_CONNECTOR_ID_NT_WN)
    };

    const std::list<std::pair<std::string, std::string>> allowedWirePairsOO{
        std::make_pair(POLE_CONNECTOR_ID_NT_WS_R, POLE_CONNECTOR_ID_NT_WS),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS_R, POLE_CONNECTOR_ID_NT_WS_R),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS, POLE_CONNECTOR_ID_NT_WS_R),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS, POLE_CONNECTOR_ID_NT_WS),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS_R, POLE_CONNECTOR_ID_NT_WA),
        std::make_pair(POLE_CONNECTOR_ID_NT_WA, POLE_CONNECTOR_ID_NT_WS_R),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS, POLE_CONNECTOR_ID_NT_WA),
        std::make_pair(POLE_CONNECTOR_ID_NT_WA, POLE_CONNECTOR_ID_NT_WS),
        std::make_pair(POLE_CONNECTOR_ID_NT_WS, POLE_CONNECTOR_ID_NT_WN)
    };

    const std::unordered_set<std::string> allowedStitchConnectors
    {
        POLE_CONNECTOR_ID_NT_WS,
        POLE_CONNECTOR_ID_NT_WS_L,
        POLE_CONNECTOR_ID_NT_WS_R,
        POLE_CONNECTOR_ID_NT_WC,
        POLE_CONNECTOR_ID_FT
    };

    auto& rightCantilevers = rightPoleHelperData->connectorByCantilevers;
    auto& leftCantilevers = leftPoleHelperData->connectorByCantilevers;

    auto fillNodeNameInfo = [&leftPoleChooser](electrification::CatenaryContextData& contextData)
    {
        auto writeNodeName =
            [&leftPoleChooser](electrification::CatenaryAttachableType attachableType,
                electrification::CatenaryAttachableLength attachableLengthType,
                CatenaryAttachableNodeName& nodeNameOutput)
        {
            if (auto name = leftPoleChooser->chooseCatenaryAttachableNodeName(
                attachableType, attachableLengthType))
            {
                nodeNameOutput[attachableLengthType] = *name;
            }
        };

        auto writeNodes = [&leftPoleChooser, &writeNodeName](electrification::CatenaryAttachableType attachableType,
                              CatenaryAttachableNodeName& nodeNameOutput)
        {
            writeNodeName(attachableType, electrification::CatenaryAttachableLength::Short,
                nodeNameOutput);
            writeNodeName(attachableType, electrification::CatenaryAttachableLength::Medium,
                nodeNameOutput);
            writeNodeName(attachableType, electrification::CatenaryAttachableLength::Long,
                nodeNameOutput);
        };

        writeNodes(electrification::CatenaryAttachableType::Dropper,
            contextData.dropperNodeName);
        writeNodes(electrification::CatenaryAttachableType::Dropper1w,
            contextData.dropperNodeName1w);
        writeNodes(electrification::CatenaryAttachableType::InsulatorDropper,
            contextData.insulatorDropperNodeName);
        writeNodes(electrification::CatenaryAttachableType::InsulatorDropper1w,
            contextData.insulatorDropperNodeName1w);
        writeNodes(electrification::CatenaryAttachableType::CompensatedMidpointAnchor,
            contextData.compMidPointAnchorName);
        writeNodes(electrification::CatenaryAttachableType::SemiCompensatedMidpointAnchor,
            contextData.scompMidPointAnchorName);
    };

    PoleSide poleSide = PoleSide::LeftHand;
    if (args.leftPole.transitionalPoleSubtype == TransitionalPoleType::TransitionalB
        || args.rightPole.transitionalPoleSubtype == TransitionalPoleType::TransitionalA)
    {
        poleSide = PoleSide::RightHand;
    }

    auto updateTransitionalCCantileversGroup =
        [&poleSide](std::unordered_map<std::string, PoleCantileverConnectors>& cantilevers,
            electrification::PoleContextData const& contextData)
    {
        if (cantilevers.size() != 2)
        {
            return;
        }

        if(contextData.transitionalPoleSubtype == TransitionalPoleType::TransitionalC ||
            contextData.transitionalPoleSubtype == TransitionalPoleType::TransitionalCInsulated)
        {
            if (contextData.registrationArmType == RegistrationArmType::StraightPullOff ||
                poleSide == PoleSide::RightHand &&
                    contextData.registrationArmType == RegistrationArmType::CurvePushOff ||
                poleSide == PoleSide::LeftHand &&
                    contextData.registrationArmType == RegistrationArmType::CurvePullOff)
            {
                auto group0 =
                    cantilevers.find(POLE_CONNECTOR_ID_NT_WS_L);
                auto group1 =
                    cantilevers.find(POLE_CONNECTOR_ID_NT_WS_R);

                if (group0 != cantilevers.end() && group1 != cantilevers.end())
                {
                    std::swap(group0->second, group1->second);
                }

            }
        }
    };

    // необходимо, так как сейчас нода для TransitionalC не меняется
    // в зависимости от фиксатора
    updateTransitionalCCantileversGroup(leftCantilevers, args.leftPole);
    updateTransitionalCCantileversGroup(rightCantilevers, args.rightPole);

    bool onlyOneConnection = true;
    std::unordered_set<std::string> busyConnectors;
    std::list<std::pair<std::string, std::string>> allowedPairs;

    if (args.leftPole.poleType == electrification::PoleType::Transitional &&
        args.rightPole.poleType == electrification::PoleType::Transitional)
    {
        allowedPairs = allowedTransionalWirePairs;
        onlyOneConnection = false;
    }
    else
    {
        if (args.leftPole.registrationArmType == RegistrationArmType::CurvePullOff ||
            args.leftPole.registrationArmType == RegistrationArmType::StraightPullOff)
        {
            if (args.rightPole.registrationArmType == RegistrationArmType::CurvePullOff ||
                args.rightPole.registrationArmType == RegistrationArmType::StraightPullOff)
            {
                allowedPairs = allowedWirePairsPP;
            }
            else
            {
                allowedPairs = allowedWirePairsPO;
            }
        }
        else if (args.leftPole.registrationArmType == RegistrationArmType::CurvePushOff ||
            args.leftPole.registrationArmType == RegistrationArmType::StraightPushOff)
        {
            if (args.rightPole.registrationArmType == RegistrationArmType::CurvePullOff ||
                args.rightPole.registrationArmType == RegistrationArmType::StraightPullOff)
            {
                allowedPairs = allowedWirePairsOP;
            }
            else
            {
                allowedPairs = allowedWirePairsOO;
            }
        }
        else
        {
            allowedPairs = allowedWirePairsCommon;
        }
    }

    bool connected = false;
    for (auto const& [leftNTKey, rightNTKey] : allowedPairs)
    {
        if (busyConnectors.count(leftNTKey) > 0)
        {
            continue;
        }

        auto findCantileverIt = leftCantilevers.find(leftNTKey);
        if (findCantileverIt == leftCantilevers.end())
        {
            continue;
        }

        PoleCantileverConnectors const& leftConnectors = findCantileverIt->second;

        auto findIt = rightCantilevers.find(rightNTKey);
        if (findIt == rightCantilevers.cend())
        {
            continue;
        }

        bool leftStitch{false};
        bool rightStitch{false};

        if (allowedStitchConnectors.count(leftNTKey) > 0)
        {
            leftStitch = args.leftStitch;
        }

        if (allowedStitchConnectors.count(rightNTKey) > 0)
        {
            rightStitch = args.rightStitch;
        }

        PoleCantileverConnectors const& rightConnectors = findIt->second;

        DynamicElectrificationCatenaryGenerator::CreateArgs catenaryGenArgs;
        catenaryGenArgs.defaultCantileverPreset = args.defaultCantileverPreset;
        catenaryGenArgs.creationMode = args.creationMode;
        catenaryGenArgs.midPointAnchorType = args.midPointAnchorType;

        if (args.leftPole.transitionalPoleSubtype && args.rightPole.transitionalPoleSubtype)
        {
            TransitionalPoleType leftType = *args.leftPole.transitionalPoleSubtype;
            TransitionalPoleType rightType = *args.rightPole.transitionalPoleSubtype;

            if ((leftType == TransitionalPoleType::TransitionalAInsulated ||
                leftType == TransitionalPoleType::TransitionalBInsulated ||
                leftType == TransitionalPoleType::TransitionalCInsulated) &&
                (rightType == TransitionalPoleType::TransitionalAInsulated ||
                rightType == TransitionalPoleType::TransitionalBInsulated ||
                rightType == TransitionalPoleType::TransitionalCInsulated))
            {
                if ((leftNTKey == POLE_CONNECTOR_ID_NT_WA ||
                        leftNTKey == POLE_CONNECTOR_ID_NT_WN) &&
                    (rightNTKey == POLE_CONNECTOR_ID_NT_WS ||
                        rightNTKey == POLE_CONNECTOR_ID_NT_WS_L ||
                        rightNTKey == POLE_CONNECTOR_ID_NT_WS_R))
                {
                    catenaryGenArgs.insulatedOverlap = true;
                }
                else if ((rightNTKey == POLE_CONNECTOR_ID_NT_WA ||
                             rightNTKey == POLE_CONNECTOR_ID_NT_WN) &&
                    (leftNTKey == POLE_CONNECTOR_ID_NT_WS ||
                        leftNTKey == POLE_CONNECTOR_ID_NT_WS_L ||
                        leftNTKey == POLE_CONNECTOR_ID_NT_WS_R))
                {
                    catenaryGenArgs.insulatedOverlap = true;
                    catenaryGenArgs.isInsulatedOverlapFlipped = true;
                }
            }
        }

        catenaryGenArgs.contextData = connectTwoCantileversByCatenary(
            ConnectTwoCantileversArgs{leftConnectors, rightConnectors, args.leftPole.transform,
                args.rightPole.transform, leftStitch, rightStitch});
        fillNodeNameInfo(catenaryGenArgs.contextData);

        result.emplace_back(std::move(catenaryGenArgs));
        busyConnectors.insert(leftNTKey);

        if (onlyOneConnection)
        {
            connected = true;
            break;
        }

    } // for (auto const& [leftNTKey, rightNTKey] : allowedTransionalWirePairs)

    return result;
}

electrification::CatenaryContextData
DynamicElectrificationCatenaryGenerator::connectTwoCantileversByCatenary(
    ConnectTwoCantileversArgs const& args)
{
    electrification::CatenaryContextData result;

    Math::Mat4 leftCableWorldTransform = args.leftTransform * args.leftCantilever.cable;
    Math::Mat4 rightCableWorldTransform = args.rightTransform * args.rightCantilever.cable;

    result.messengerWire.startPosition = leftCableWorldTransform.getTranslate();
    result.messengerWire.endPosition = rightCableWorldTransform.getTranslate();

    Math::Vec3 leftAxisY = args.leftTransform.getAxisY();
    Math::Vec3 rightAxisY = args.rightTransform.getAxisY();

    double dot = Math::dot(leftAxisY, rightAxisY);
    bool swapContactWires = DefaultDoubleCompare::moreLess(dot, 0.0);

    if (args.rightCantilever.wireFar &&  args.leftCantilever.wireFar)
    {
        electrification::CatenaryWireData wireData;

        Math::Mat4 leftWireWorldTransform = args.leftTransform * (*args.leftCantilever.wireFar);
        Math::Mat4 rightWireWorldTransform;
        if (swapContactWires && args.leftCantilever.wireNear)
        {
            rightWireWorldTransform =
                args.rightTransform * (*args.rightCantilever.wireNear);
        }
        else
        {
            rightWireWorldTransform =
                args.rightTransform * (*args.rightCantilever.wireFar);
        }

        wireData.startPosition = leftWireWorldTransform.getTranslate();
        wireData.endPosition = rightWireWorldTransform.getTranslate();

        result.contactWires.emplace(ELECTRIFICATION_POLE_WIRE_CONNECTOR_FAR_INDEX, std::move(wireData));
    }

    if (args.rightCantilever.wireNear && args.leftCantilever.wireNear)
    {
        electrification::CatenaryWireData wireData;

        Math::Mat4 leftWireWorldTransform = args.leftTransform * (*args.leftCantilever.wireNear);
        Math::Mat4 rightWireWorldTransform;
        if (swapContactWires && args.rightCantilever.wireFar)
        {
            rightWireWorldTransform = args.rightTransform * (*args.rightCantilever.wireFar);
        }
        else
        {
            rightWireWorldTransform = args.rightTransform * (*args.rightCantilever.wireNear);
        }

        wireData.startPosition = leftWireWorldTransform.getTranslate();
        wireData.endPosition = rightWireWorldTransform.getTranslate();

        result.contactWires.emplace(ELECTRIFICATION_POLE_WIRE_CONNECTOR_NEAR_INDEX, std::move(wireData));
    }

    result.leftStitch = args.leftStitch;
    result.rightStitch = args.rightStitch;

    return result;
}

DynamicElectrificationCatenaryGenerator::DynamicElectrificationCatenaryGenerator(uint32_t id,
    CreateArgs&& args)
    : DynamicElectrificationPart(id, 0)
    , m_genParams(std::move(args))
{
    auto generationHost = DynamicElectrification::instance();
    assert(generationHost);
    auto cantileverPresetConfig =
        generationHost->getOrCreateMeshConfig(m_genParams.defaultCantileverPreset);
    assert(cantileverPresetConfig);

    auto& catenaryData = cantileverPresetConfig->getCatenaryData();
    auto& portalData = cantileverPresetConfig->getPortalData();

    m_contactRadius = catenaryData.contactWireDiameter * 0.5;
    m_messengerRadius = catenaryData.catenaryWireDiameter * 0.5;
    m_dropperRadius = catenaryData.dropperDiameter * 0.5;
    if (m_genParams.isCrossSpan)
    {
        m_saggingCoefficient = portalData.crossWireSaggingCoefficient;
        m_spanConnectorOffset = portalData.spanWireConnectorOffset;
    }
    else
    {
        m_saggingCoefficient = catenaryData.saggingCoefficient;
    }

    m_insulatorDropperStepM = catenaryData.overlapInsulatorDtopperStepM;
    m_numInsulatorDroppers = catenaryData.numOverlapInsulatorDroppers;

    m_useMeshDropper = catenaryData.useMeshDropper;
    m_mainWireId = ELECTRIFICATION_POLE_WIRE_CONNECTOR_NEAR_INDEX;

    if (m_genParams.isSpanWire)
    {
        return;
    }

    if (!catenaryData.sectionInsulator.nodeUpper.empty())
    {
        InsulatorPtr insulator = std::make_shared<Insulator>(
            catenaryData.sectionInsulator.nodeUpper, catenaryData.sectionInsulator.distance,
            catenaryData.sectionInsulator.useDroppers, /*isFlipped=*/false);
        m_insulatorsByType.emplace(InsulatorPositionType::SectionUpper, insulator);
    }

    if (!catenaryData.sectionInsulator.nodeLower.empty())
    {
        InsulatorPtr insulator =
            std::make_shared<Insulator>(
                catenaryData.sectionInsulator.nodeLower, catenaryData.sectionInsulator.distance,
            catenaryData.sectionInsulator.useDroppers, /*isFlipped=*/false);
        m_insulatorsByType.emplace(InsulatorPositionType::SectionLower, insulator);
        if (catenaryData.sectionInsulator.useDroppers)
        {
            auto upperInsulator = m_insulatorsByType.find(InsulatorPositionType::SectionUpper);
            if (upperInsulator != m_insulatorsByType.end())
            {
                insulator->connectOtherAttachable(upperInsulator->second);
            }
        }
    }

    if (!catenaryData.tensionInsulator.nodeUpper.empty())
    {
        InsulatorPtr insulator =
            std::make_shared<Insulator>(
                catenaryData.tensionInsulator.nodeUpper, catenaryData.tensionInsulator.distance,
            catenaryData.tensionInsulator.useDroppers, /*isFlipped=*/false);
        m_insulatorsByType.emplace(InsulatorPositionType::TensionUpper, insulator);
    }

    if (!catenaryData.tensionInsulator.nodeLower.empty())
    {
        InsulatorPtr insulator = 
            std::make_shared<Insulator>(
                catenaryData.tensionInsulator.nodeLower, catenaryData.tensionInsulator.distance,
            catenaryData.tensionInsulator.useDroppers, /*isFlipped=*/false);
        m_insulatorsByType.emplace(InsulatorPositionType::TensionLower, insulator);
    }

    if (m_genParams.insulatedOverlap)
    {
        double distance = catenaryData.overlapInsulator.distance;
        if (m_genParams.isInsulatedOverlapFlipped)
        {
            double length = (m_genParams.contextData.messengerWire.endPosition -
                m_genParams.contextData.messengerWire.startPosition).length();
            distance = length - distance;
        }

        if (distance <= 0.0)
        {
            return;
        }

        if (!catenaryData.overlapInsulator.nodeUpper.empty())
        {
            InsulatorPtr insulator = std::make_shared<Insulator>(
                catenaryData.overlapInsulator.nodeUpper, distance,
                catenaryData.overlapInsulator.useDroppers, /*isFlipped=*/false);
            m_insulatorsByType.emplace(InsulatorPositionType::OverlapUpper, insulator);
        }

        if (!catenaryData.overlapInsulator.nodeLower.empty())
        {
            InsulatorPtr insulator = std::make_shared<Insulator>(
                catenaryData.overlapInsulator.nodeLower, distance,
                catenaryData.overlapInsulator.useDroppers, /*isFlipped=*/false);
            m_insulatorsByType.emplace(InsulatorPositionType::OverlapLower, insulator);

            if (catenaryData.overlapInsulator.useDroppers)
            {
                auto upperInsulator =
                    m_insulatorsByType.find(InsulatorPositionType::OverlapUpper);
                if (upperInsulator != m_insulatorsByType.end())
                {
                    insulator->connectOtherAttachable(upperInsulator->second);
                }
            }
        }
    }

    if (m_genParams.isCrossSpan)
    {
        m_dropperClampNodeName = portalData.dropperClamp;

        auto crossWireInsulatorNode = portalData.crossWireInsulatorNode;
        double length = (m_genParams.contextData.messengerWire.endPosition -
                m_genParams.contextData.messengerWire.startPosition).length();

        if (!crossWireInsulatorNode.empty())
        {
            InsulatorPtr insulator0 = std::make_shared<Insulator>(
                crossWireInsulatorNode, /*distance=*/0.0,
                /*useDroppers=*/false, /*isFlipped=*/false);
            m_insulatorsByType.emplace(InsulatorPositionType::TensionUpper, insulator0);

            InsulatorPtr insulator1 = std::make_shared<Insulator>(
                crossWireInsulatorNode, length,
                /*useDroppers=*/false, /*isFlipped=*/true);
            m_insulatorsByType.emplace(InsulatorPositionType::TensionUpper, insulator1);
        }
    }
}

bool
DynamicElectrificationCatenaryGenerator::load()
{
    m_mesh = Mesh::create();

    generateContactWires();
    generateMessengerWire();
    assert(m_messengerWire);
    if (!m_messengerWire)
    {
        return false;
    }

    // fill droppers params
    if (m_dropperParams.empty() && m_genParams.dropperPositions.empty())
    {
        if (m_genParams.insulatedOverlap)
        {
            std::vector<double> dropperParams = getDropperParams();
            std::vector<double> insulatorDropperParams = getInsulatedOverlapDropperParams();
            if (insulatorDropperParams.size() > 0)
            {
                const double dropperStepOffset =
                    m_messengerWire->getParamByDistance(m_insulatorDropperStepM);

                auto minMax =
                    std::minmax(insulatorDropperParams.begin(), insulatorDropperParams.end());
                const double minParam = *std::min_element(insulatorDropperParams.begin(),
                    insulatorDropperParams.end()) - dropperStepOffset;
                const double maxParam = *std::max_element(insulatorDropperParams.begin(),
                    insulatorDropperParams.end()) + dropperStepOffset;

                for (double param : dropperParams)
                {
                    if (param < minParam || param > maxParam)
                    {
                        m_dropperParams.push_back(param);
                    }
                }

                m_dropperParams.insert(m_dropperParams.end(),
                    insulatorDropperParams.begin(), insulatorDropperParams.end());
            }
            else
            {
                m_dropperParams = dropperParams;
            }
        }
        else
        {
            m_dropperParams = getDropperParams();
        }
    }

    generateMidPointAnchor();

    if (m_genParams.dropperPositions.empty())
    {
        generateDroppers();
    }
    else
    {
        calcDroppersFromPositions();
    }

    generateStitchWires();

    auto nearWireFind = m_contactWires.find(ELECTRIFICATION_POLE_WIRE_CONNECTOR_NEAR_INDEX);
    if (nearWireFind != m_contactWires.end())
    {
        WirePtr nearWire = nearWireFind->second;
        auto& insulators = nearWire->getMorticeAttachables();

        for (auto const& [param, insulator] : insulators)
        {
            if (insulator->useDroppers())
            {
                generateInsulatorDroppers(insulator);
            }
        }
    }

    if (m_genParams.isCrossSpan)
    {
        m_messengerWire->straighten();
        generateSpanWireConnector();
    }

    addMeshPart(m_messengerWire->getMesh(), "messenger_wire");

    for (auto& [connectorId, contactWire] : m_contactWires)
    {
        std::string surfaceName = "contact_wire_" + std::to_string(connectorId);
        addMeshPart(contactWire->getMesh(), surfaceName);
    }

    m_mesh->createBounds();

    return true;
}

void
DynamicElectrificationCatenaryGenerator::unload()
{
    m_mesh.deleteLater();
    m_messengerWire.reset();
    m_contactWires.clear();
}

Unigine::NodePtr
DynamicElectrificationCatenaryGenerator::createNodes()
{
    maybeCreateRootNode();

    auto generationHost = DynamicElectrification::instance();
    assert(generationHost);
    auto cantileverPresetConfig =
        generationHost->getOrCreateMeshConfig(m_genParams.defaultCantileverPreset);
    assert(cantileverPresetConfig);

    std::string materialStr{ELECTRIFICATION_DEFAULT_CATENARY_MATERIAL};
    if (cantileverPresetConfig)
    {
        materialStr = cantileverPresetConfig->getCatenaryData().wireMaterial;
    }

    ObjectMeshStaticPtr wiresNode = ObjectMeshStatic::create(m_mesh);
    MaterialPtr material = Materials::findMaterial(materialStr.c_str());
    if (material)
    {
        wiresNode->setMaterial(material, "*");
    }
    else
    {
        error("Material %s not found\n", materialStr);
        MaterialPtr mesh_base = Materials::findMaterial("mesh_base");
        wiresNode->setMaterial(mesh_base, "*");
    }

    if (cantileverPresetConfig)
    {
        for (int i = 0; i < wiresNode->getNumSurfaces(); i++)
        {
            wiresNode->setMaxVisibleDistance(
                cantileverPresetConfig->getCatenaryData().maxVisibilityDistance, i);
            wiresNode->setMaxFadeDistance(
                cantileverPresetConfig->getCatenaryData().maxFadeDistance, i);
            wiresNode->setCastShadow(false, i);
            wiresNode->setCastWorldShadow(false, i);
            wiresNode->setShadowMask(0x00, i);
        }
    }

    m_rootNode->addChild(wiresNode);
    if (m_genParams.creationMode != electrification::CreationMode::NODES)
    {
        return m_rootNode;
    }

    std::function<void(std::string const&, Unigine::Math::Mat4 const&)> addChildNode =
        [this](std::string const& nodeName, Unigine::Math::Mat4 const& transform)
    {
        if (nodeName.empty())
        {
            return;
        }

        NodePtr node = World::loadNode(nodeName.c_str());
        if (node)
        {
            m_rootNode->addChild(node);
            Math::Mat4 insulatorTransform = transform;
            node->setWorldTransform(globalTransform() * insulatorTransform);
        }
    };

    addNodes(addChildNode);
    return m_rootNode;
}

void
DynamicElectrificationCatenaryGenerator::validateClusters(
    GetOrCreateClusterFunc const& getOrCreateCluster,
    std::optional<Unigine::Math::Vec3> const& center) const
{
    if (m_genParams.creationMode != electrification::CreationMode::CLUSTER)
    {
        return;
    }

    Math::Mat4 globalT = globalTransform();

    std::function<void(std::string const&, Unigine::Math::Mat4 const&)> addClusterNodeInstance =
        [&getOrCreateCluster, &center, &globalT](std::string const& nodeName, Math::Mat4 const& localTransform)
    {
        if (auto cluster = getOrCreateCluster(nodeName,
                /*matInfo=*/RailwayMeshGeneratorConfig::MaterialInfo{}))
        {
            if (center)
            {
                double distance = (globalT * localTransform.getTranslate() - (*center)).length();
                double visibleDistance = cluster->getVisibleDistance() * 2.0f;
                if (distance > visibleDistance)
                {
                    return;
                }
            }

            const Math::Mat4 invClusterTransform = Math::inverse(cluster->getWorldTransform());
            const Math::Mat4 localClusterTransform =
                invClusterTransform * (globalT * localTransform);
            const int instanceIdx = cluster->addMeshTransform();
            cluster->setMeshTransform(instanceIdx, Math::mat4{localClusterTransform});
        }
    };

    addNodes(addClusterNodeInstance);
}

void
DynamicElectrificationCatenaryGenerator::generateInsulatorDroppers(MorticeAttachablePtr insulator)
{
    MorticeAttachablePtr connectedInsulator = insulator->getConnectedInsulator();
    if (!connectedInsulator)
    {
        return;
    }

    NodeLengthMap lengthMap = getNodeLengths(
        m_genParams.contextData.insulatorDropperNodeName);

    auto createDropper = [this, &insulator, &connectedInsulator,
                             &lengthMap](Math::Vec3 const& lowerPos,
                             Math::Vec3 const& upperPos, int tangentSign)
    {
        Math::Mat4 lowerTransform = insulator->getWorldTransform();
        Math::Mat4 upperTransform = connectedInsulator->getWorldWithNoInitialTransform();
        Math::Vec3 upVector = upperTransform * lowerPos - lowerTransform * lowerPos;

        // нулевая точка изолятора находится не ровно под точкой его крепления к верхней части,
        // из-за чего при наклонённом изоляторе может неверно рассчитываться её высота над проводом;
        // немного откорректируем позицию, чтобы немного нивелировать эту разницу
        Math::Vec3 upperPosition = upperTransform.getTranslate();
        Math::Vec3 lowerPosition = lowerTransform.getTranslate();
        double offset = (lowerPosition - lowerTransform * lowerPos).length();
        Math::Vec3 upperPosCorrected =
            upperPosition - upperTransform.getAxisX() * offset * tangentSign;
        double length = (upperPosCorrected - lowerTransform * lowerPos).z;

        upVector.normalize();

        Math::Vec3 tangent = lowerTransform.getAxisX() * tangentSign;
        Math::Vec3 axisY = Math::cross(upVector, tangent);
        Math::Mat4 dropperTransform{tangent, axisY, upVector, lowerTransform * lowerPos};

        std::optional<electrification::CatenaryAttachableLength> lengthType =
            chooseNodeLengthType(lengthMap, length);
        if (!lengthType)
        {
            return;
        }

        double nodeLength = lengthMap[*lengthType];
        double lengthScale = length / nodeLength;
        Math::Mat4 scale = Math::scale(Math::Vec3(1., 1., lengthScale));

        AttachableInfo dropperInfo;
        dropperInfo.type = electrification::CatenaryAttachableType::InsulatorDropper;
        dropperInfo.lengthType = *lengthType;
        dropperInfo.transform = dropperTransform * scale;

        m_attachableTransforms.push_back(dropperInfo);
    };

    auto dropperInPosition = insulator->getConnectorPosition(AttachableConnectorType::KP_DROPPER_IN);
    auto dropperOutPosition =
        insulator->getConnectorPosition(AttachableConnectorType::KP_DROPPER_OUT);
    auto dropperInPositionUpper =
        connectedInsulator->getConnectorPosition(AttachableConnectorType::NT_IN);
    auto dropperOutPositionUpper =
        connectedInsulator->getConnectorPosition(AttachableConnectorType::NT_OUT);

    assert(dropperInPosition && dropperOutPosition && dropperInPositionUpper &&
        dropperOutPositionUpper);
    if (!dropperInPosition || !dropperOutPosition || !dropperInPositionUpper ||
        !dropperOutPositionUpper)
    {
        return;
    }

    createDropper(*dropperInPosition, *dropperInPositionUpper, /*tangentSign=*/1);
    createDropper(*dropperOutPosition, *dropperOutPositionUpper, /*tangentSign=*/-1);
}

void
DynamicElectrificationCatenaryGenerator::generateMessengerWire()
{
    const Math::Mat4 localTransform = toLocalTransform();
    Wire::CreateArgs args;
    args.wireType = Wire::WireType::Nt;
    args.startPos = localTransform * m_genParams.contextData.messengerWire.startPosition;
    args.endPos = localTransform * m_genParams.contextData.messengerWire.endPosition;
    args.radius = m_messengerRadius;
    args.saggingOffset = maxOffset();

    if (m_genParams.isSpanWire)
    {
        args.saggingOffset = 0.0;
    }

    if (m_genParams.isCrossSpan)
    {
        args.straightenedWire = true;
    }

    try
    {
        m_messengerWire = std::make_shared<Wire>(std::move(args));
    }
    catch (std::exception const& e)
    {
        error("Unable to create messenger wire for object %d.\n", getKey().id);
        error("Error message: %s\n", e.what());
    }

    if (!m_messengerWire)
    {
        return;
    }

    auto range = m_insulatorsByType.equal_range(InsulatorPositionType::SectionUpper);
    for (auto it = range.first; it != range.second; ++it)
    {
        m_messengerWire->addInsulator(it->second);
    }

    range = m_insulatorsByType.equal_range(InsulatorPositionType::TensionUpper);
    for (auto it = range.first; it != range.second; ++it)
    {
        m_messengerWire->addInsulator(it->second);
    }

    range = m_insulatorsByType.equal_range(InsulatorPositionType::OverlapUpper);
    for (auto it = range.first; it != range.second; ++it)
    {
        m_messengerWire->addInsulator(it->second);
    }

    m_messengerWire->update();
    generateSpanWireInsulators();
}

void
DynamicElectrificationCatenaryGenerator::generateContactWires()
{
    const Math::Mat4 localTransform = toLocalTransform();

    Math::Vec3 middlePos = Math::Vec3::ZERO;
    for (auto& [connectorId, wireData] : m_genParams.contextData.contactWires)
    {
        middlePos += wireData.startPosition;
    }

    middlePos /= m_genParams.contextData.contactWires.size();
    middlePos = localTransform * middlePos;

    for (auto& [connectorId, wireData] : m_genParams.contextData.contactWires)
    {
        Wire::CreateArgs args;
        args.startPos = localTransform * wireData.startPosition;
        args.endPos = localTransform * wireData.endPosition;
        args.offsetFromMiddle = middlePos - args.startPos;
        args.radius = m_messengerRadius;

        Wire::WireType wireType{Wire::WireType::Default};
        if (connectorId == ELECTRIFICATION_POLE_WIRE_CONNECTOR_NEAR_INDEX)
        {
            wireType = Wire::WireType::KpNear;
        }
        else if (connectorId == ELECTRIFICATION_POLE_WIRE_CONNECTOR_FAR_INDEX)
        {
            wireType = Wire::WireType::KpFar;
        }

        args.wireType = wireType;

        WirePtr contactWire;
        try
        {
            contactWire = std::make_shared<Wire>(std::move(args));
        }
        catch (std::exception const& e)
        {
            error("Unable to create contact wire for object %d.\n", getKey().id);
            error("Error message: %s\n", e.what());
        }

        if (!contactWire)
        {
            continue;
        }

        if (wireType != Wire::WireType::Default)
        {
            auto range = m_insulatorsByType.equal_range(InsulatorPositionType::SectionLower);
            for (auto it = range.first; it != range.second; ++it)
            {
                contactWire->addInsulator(it->second);
            }

            range = m_insulatorsByType.equal_range(InsulatorPositionType::TensionLower);
            for (auto it = range.first; it != range.second; ++it)
            {
                contactWire->addInsulator(it->second);
            }

            range = m_insulatorsByType.equal_range(InsulatorPositionType::OverlapLower);
            for (auto it = range.first; it != range.second; ++it)
            {
                contactWire->addInsulator(it->second);
            }
        }

        contactWire->update();
        m_contactWires.emplace(connectorId, std::move(contactWire));
    }
}

void
DynamicElectrificationCatenaryGenerator::generateSpanWireInsulators()
{
    if (!m_genParams.isSpanWire || m_genParams.spanWireSuspensionDistances.size() < 2)
    {
        return;
    }

    if (!m_messengerWire)
    {
        return;
    }

    auto generationHost = DynamicElectrification::instance();
    assert(generationHost);
    auto cantileverPresetConfig =
        generationHost->getOrCreateMeshConfig(m_genParams.defaultCantileverPreset);
    assert(cantileverPresetConfig);

    double length = m_messengerWire->airLength();
    auto& portalData = cantileverPresetConfig->getPortalData();

    auto addInsulatorAtDistance = [this, &portalData, &length](
        double distance)
    {
        InsulatorPtr insulator = std::make_shared<Insulator>(portalData.spanWireInsulatorNode, distance,
            /*useDroppers=*/false,
            /*isFlipped=*/ distance > length * 0.5);
        m_insulatorsByType.emplace(InsulatorPositionType::Reg, insulator);
        m_messengerWire->addInsulator(insulator);
    };

    auto it = m_genParams.spanWireSuspensionDistances.begin();
    double prevDistance = length * 0.5 + *it;
    if (prevDistance > portalData.spanWireInsulatorDoubleDistance)
    {
        addInsulatorAtDistance(prevDistance - portalData.spanWireInsulatorOffset);
    }

    ++it;
    double currentDistance = prevDistance;
    for (; it != m_genParams.spanWireSuspensionDistances.end(); ++it)
    {
        currentDistance = length * 0.5 + *it;
        double distance = currentDistance - prevDistance;
        if (distance > portalData.spanWireInsulatorDoubleDistance)
        {
            double firstInsulatorDistance = prevDistance + portalData.spanWireInsulatorOffset;
            double secondInsulatorDistance = currentDistance - portalData.spanWireInsulatorOffset;
            addInsulatorAtDistance(firstInsulatorDistance);
            addInsulatorAtDistance(secondInsulatorDistance);
        }
        else if (distance > portalData.spanWireInsulatorTrackDistance)
        {
            double insulatorDistance = (prevDistance + currentDistance) * 0.5;
            addInsulatorAtDistance(insulatorDistance);
        }

        prevDistance = currentDistance;
    }

    if ((length - currentDistance) > portalData.spanWireInsulatorDoubleDistance)
    {
        addInsulatorAtDistance(currentDistance + portalData.spanWireInsulatorOffset);
    }
}

void
DynamicElectrificationCatenaryGenerator::generateDroppers()
{
    /**
     *@note called from separate thread inside of load() method only
     */

    const double distanceEps = 0.005;

    if (m_dropperParams.empty() || !m_messengerWire || m_contactWires.empty())
    {
        return;
    }

    auto nearIt = m_contactWires.find(ELECTRIFICATION_POLE_WIRE_CONNECTOR_NEAR_INDEX);
    auto farIt = m_contactWires.find(ELECTRIFICATION_POLE_WIRE_CONNECTOR_FAR_INDEX);

    NodeLengthMap lengthMap = getNodeLengths(m_genParams.contextData.dropperNodeName);
    int dropperId = 0;

    if (m_contactWires.size() < 2)
    {
        WirePtr contactWire{nullptr};
        if (nearIt != m_contactWires.end())
        {
            contactWire = nearIt->second;
        }
        else if (farIt != m_contactWires.end())
        {
            contactWire = farIt->second;
        }

        if (!contactWire)
        {
            return;
        }

        for (double tParam : m_dropperParams)
        {
            std::optional<Math::Vec3> messengerPos = m_messengerWire->eval(tParam);
            if (!messengerPos)
            {
                continue;
            }

            const double contactParam = contactWire->getParamByPositionOnVerticalLine(*messengerPos);
            if (std::optional<Math::Vec3> contactPos = contactWire->eval(contactParam))
            {
                addDropper(dropperId, *messengerPos, *contactPos, /*force1w=*/false, lengthMap);
            }
        }
    }
    else
    {
        if (nearIt == m_contactWires.end() || farIt == m_contactWires.end())
        {
            return;
        }

        WirePtr nearWire = nearIt->second;
        WirePtr farWire = farIt->second;
        if (!nearWire || !farWire)
        {
            return;
        }

        double wireDistance = (nearWire->startPosition() - farWire->startPosition()).length();
        wireDistance += distanceEps;

        for (double tParam : m_dropperParams)
        {
            std::optional<Math::Vec3> messengerPos = m_messengerWire->eval(tParam);
            if (!messengerPos)
            {
                continue;
            }

            const double nearParam = nearWire->getParamByPositionOnVerticalLine(*messengerPos);
            std::optional<Math::Vec3> nearPos = nearWire->eval(nearParam);

            const double farParam = farWire->getParamByPositionOnVerticalLine(*messengerPos);
            std::optional<Math::Vec3> farPos = farWire->eval(farParam);

            if (!nearPos || !farPos)
            {
                continue;
            }

            const double distance = (*nearPos - *farPos).length();
            if (distance > wireDistance)
            {
                Math::Vec3 dropperOffset =
                    Math::Vec3::FORWARD * ELECTRIFICATION_FORCED_1W_DROPPER_OFFSET;
                addDropper(dropperId, *messengerPos + dropperOffset, *nearPos + dropperOffset,
                    /*force1w=*/true, lengthMap);
                addDropper(dropperId, *messengerPos - dropperOffset, *farPos - dropperOffset,
                    /*force1w=*/true, lengthMap);
            }
            else
            {
                Math::Vec3 contactPos = (*nearPos + *farPos) * 0.5;
                addDropper(dropperId, *messengerPos, contactPos, /*force1w=*/false, lengthMap);
            }
        }
    }
}

void
DynamicElectrificationCatenaryGenerator::calcDroppersFromPositions()
{
    /**
     *@note called from separate thread inside of load() method only
     */

    const double distanceEps = 0.005;

    if (!m_messengerWire || m_contactWires.empty())
    {
        return;
    }

    NodeLengthMap lengthMap = getNodeLengths(m_genParams.contextData.dropperNodeName);
    int dropperId = 0;

    Math::Mat4 local = toLocalTransform();

    for (auto const& contactPos : m_genParams.dropperPositions)
    {
        Math::Vec3 localContactPos = local * contactPos;
        const double messengerParam = m_messengerWire->getParamByPositionOnVerticalLine(localContactPos);
        if (std::optional<Math::Vec3> messengerPos = m_messengerWire->eval(messengerParam))
        {
            if (m_genParams.isCrossSpan)
            {
                double distance = m_messengerWire->length() * messengerParam;

                Math::Vec3 newPos{localContactPos.x, localContactPos.y, messengerPos->z};
                DropperClampPtr dropperClamp =
                    std::make_shared<DropperClamp>(m_dropperClampNodeName, distance);
                m_messengerWire->addDropperClamp(
                    dropperClamp, newPos, messengerParam);

                m_dropperParams.push_back(messengerParam);

                addDropper(dropperId, newPos, localContactPos, /*force1w=*/false, lengthMap);
            }
            else
            {
                addDropper(dropperId, *messengerPos, localContactPos, /*force1w=*/false, lengthMap);
            }
        }
    }
}

void
DynamicElectrificationCatenaryGenerator::generateStitchWires()
{
    if (!m_genParams.contextData.leftStitch && !m_genParams.contextData.rightStitch
        || m_genParams.contextData.isAnchor)
    {
        return;
    }

    if (!m_messengerWire || m_contactWires.empty())
    {
        return;
    }

    NodeLengthMap lengthMap = getNodeLengths(m_genParams.contextData.dropperNodeName);
    auto generateStitch = [&](
        double tContactParam, double distance,
        Math::Vec3 const& messengerPolePos,
        Math::Vec3 const& contactPolePos,
        std::string sideName)
    {
        const double tMessenger = m_messengerWire->getParamByDistance(distance);
        std::optional<Math::Vec3> messengerPos = m_messengerWire->eval(tMessenger);
        if (!messengerPos)
        {
            return;
        }

        std::optional<Math::Vec3> contactPos;
        int posFound = 0;
        for (auto& [connectorId, contactWire] : m_contactWires)
        {
            std::optional<Math::Vec3> wirePos = contactWire->eval(tContactParam);
            if (contactPos)
            {
                if (wirePos)
                {
                    posFound += 1;
                    contactPos = *contactPos + *wirePos;
                }
            }
            else
            {
                contactPos = wirePos;
            }
        }

        if (!contactPos)
        {
            return;
        }

        posFound += 1;
        contactPos = *contactPos / posFound;

        double offset = ELECTRIFICATION_DEFAULT_CATENARY_HEIGHT * 0.5;
        while (offset > std::abs(messengerPolePos.z - contactPolePos.z))
        {
            offset *= 0.5;
        }

        const double coefficient = offset / std::abs(messengerPolePos.z - contactPolePos.z);
        Math::Vec3 stitchPolePos =
            messengerPolePos + (contactPolePos - messengerPolePos) * coefficient;

        Math::Vec3 flyingPoint =
            (messengerPolePos - contactPolePos) * (1.0 - coefficient) + *contactPos;

        generateStraightWire(flyingPoint, stitchPolePos,
            /*addConnector=*/false, "horisontal_spring_" + sideName);
        generateStraightWire(flyingPoint, *messengerPos,
            /*addConnector=*/false, "diagonal_spring_" + sideName);

        Math::Vec3 dropperDirection = flyingPoint  - * contactPos;
        double dropperHeight = dropperDirection.length();
        std::optional<electrification::CatenaryAttachableLength> lengthType =
            chooseNodeLengthType(lengthMap, dropperHeight);
        if (lengthType)
        {
            addDropper(0, flyingPoint, *contactPos, /*force1w=*/false, lengthMap);
        }
        else
        {
            generateStraightWire(flyingPoint, *contactPos,
                /*addConnector=*/true, "vertical_spring_" + sideName);

            for (auto& [wireId, wire] : m_contactWires)
            {
                if (wireId == m_mainWireId)
                {
                    continue;
                }

                const double wireParam = wire->getParamByPositionOnVerticalLine(*contactPos);
                std::optional<Math::Vec3> wireConnectorPos = wire->eval(wireParam);
                if (!wireConnectorPos)
                {
                    continue;
                }

                double distance = (*wireConnectorPos - *contactPos).length();
                if (distance > ELECTRIFICATION_DROPPER_LENGTH_MAX)
                {
                    continue;
                }

                generateWireConnector(*contactPos, *wireConnectorPos,
                    "stitch_connector_" + std::to_string(wireId));
            }
        }
    };

    Math::Vec3 contactStart, contactEnd;
    for (auto& [connectorId, contactWire] : m_contactWires)
    {
        contactStart += contactWire->startPosition();
        contactEnd += contactWire->endPosition();
    }

    if (!m_contactWires.empty())
    {
        contactStart = contactStart / m_contactWires.size();
        contactEnd = contactEnd / m_contactWires.size();
    }

    // START
    if (m_genParams.contextData.leftStitch)
    {
        const Math::Vec3 messengerStart = m_messengerWire->startPosition();
        const double leftParam =
            m_messengerWire->getParamByDistance(ELECTRIFICATION_STITCH_DISTANCE);
        const double distanceLeft = ELECTRIFICATION_STITCH_CONNECTOR_DISTANCE;

        generateStitch(leftParam, distanceLeft, messengerStart, contactStart, "left");
    }

    //END
    if (m_genParams.contextData.rightStitch)
    {
        const Math::Vec3 messengerEnd = m_messengerWire->endPosition();
        const double contactLength = (contactEnd - contactStart).length();
        const double rightParam =
            m_messengerWire->getParamByDistance(contactLength - ELECTRIFICATION_STITCH_DISTANCE);
        const double distanceRight = contactLength - ELECTRIFICATION_STITCH_CONNECTOR_DISTANCE;

        generateStitch(rightParam, distanceRight, messengerEnd, contactEnd, "right");
    }
}

void
DynamicElectrificationCatenaryGenerator::generateWireConnector(Math::Vec3 const& startPos,
    Math::Vec3 const& endPos, std::string const& surfaceName)
{
    static double seed = 17;

    Math::Vec3 direction = endPos - startPos;
    Math::Vec3 p0 = Math::Vec3::ZERO;
    Math::Vec3 p3 = Math::Vec3::FORWARD * direction.length();

    Math::Vec3 rightTangent = p3;
    Math::Vec3 leftTangent = -rightTangent;

    std::mt19937 random(getKey().id + getKey().subId + seed);

    auto randomDouble = [&random](double from, double to)
    {
        std::uniform_real_distribution<double> addDouble(from, to);
        double result = addDouble(random);
        return result;
    };

    seed *= randomDouble(0.005, 17.);

    double maxZ = direction.length() * 0.6;
    double maxX = direction.length() * 0.2;
    double maxY = direction.length();
    double minY = direction.length() * (-0.2);

    rightTangent.x = randomDouble(-maxX, maxX);
    leftTangent.x = randomDouble(-maxX, maxX);

    rightTangent.y = randomDouble(minY, maxY);
    leftTangent.y = randomDouble(minY, maxY);

    rightTangent.z = randomDouble(0., maxZ);
    leftTangent.z = randomDouble(0., maxZ);

    auto randomLength = [&random]()
    {
        std::uniform_real_distribution<double> addLength(0.5, 1.0);
        double lengthMod = addLength(random);
        return lengthMod;
    };

    BezierSegment segment(bezierSegmentByPositionsAndTangents(
        p0, p3, rightTangent, leftTangent));

    Math::mat4 basis;
    if (std::abs(direction.x) < 1e-6 && std::abs(direction.y) < 1e-6)
    {
        basis = getVerticalTransform(startPos);
    }
    else
    {
        basis = Math::mat4(transformByTwoPoints(startPos, endPos));
    }

    MeshPtr mesh = Mesh::create();

    Wire::generateWireMeshFromSpline(segment, m_dropperRadius, "wire", mesh);
    mesh->setSurfaceTransform(basis);
     int startId =
        mesh->addSphereSurface("start_surface", m_messengerRadius * 1.2, 6, 10);
    int endId = mesh->addSphereSurface("end_surface", m_contactRadius * 1.2, 6, 10);

    mesh->setSurfaceTransform(Math::mat4(Math::translate(startPos)), startId);
    mesh->setSurfaceTransform(Math::mat4(Math::translate(endPos)), endId);

    addMeshPart(mesh, surfaceName);
}

void
DynamicElectrificationCatenaryGenerator::generateMidPointAnchor()
{
    if (!m_messengerWire || m_dropperParams.empty())
    {
        return;
    }

    if (m_genParams.midPointAnchorType == MidPointAnchorType::NONE)
    {
        return;
    }

    std::optional<Math::Mat4> midPointAnchorTransform;

    double lengthScaleX = 1.;
    std::optional<Math::Vec3> messengerPos;
    if (m_genParams.midPointAnchorType == MidPointAnchorType::MIDDLE)
    {
        double insertParam = 0.5;
        std::optional<Math::Vec3> insertPos = getAverageContactWirePos(insertParam);
        messengerPos = m_messengerWire->eval(insertParam);

        assert(insertPos && messengerPos);
        if (!insertPos || !messengerPos)
        {
            return;
        }

        Math::Vec3 directionZ = *messengerPos - *insertPos;
        Math::Vec3 axisZ = directionZ;
        axisZ.normalize();

        Math::Vec3 axisY = Math::cross(axisZ, Math::Vec3::FORWARD);
        axisY.normalize();

        Math::Mat4 rotate{Math::Vec3::FORWARD, axisY, axisZ, Math::Vec3::ZERO};
        midPointAnchorTransform = Math::translate(*insertPos) * rotate;
    }
    else
    {
        int numDroppers = m_dropperParams.size();
        if (numDroppers < 3)
        {
            return;
        }

        double startParam, endParam;
        if (m_genParams.midPointAnchorType == MidPointAnchorType::START)
        {
            startParam = m_dropperParams[0];
            endParam = m_dropperParams[1];
        }
        else //if (m_genParams.midPointAnchorType == MidPointAnchorType::END)
        {
            endParam = m_dropperParams[numDroppers - 2];
            startParam = m_dropperParams[numDroppers - 1];
        }

        std::optional<Math::Vec3> startPos = getAverageContactWirePos(startParam);
        std::optional<Math::Vec3> endPos = getAverageContactWirePos(endParam);
        messengerPos = m_messengerWire->eval(startParam);

        if (!startPos || !endPos || !messengerPos)
        {
            return;
        }

        Math::Vec3 directionX = *endPos - *startPos;
        Math::Vec3 axisX = directionX;
        axisX.normalize();

        Math::Vec3 directionZ = *messengerPos - *startPos;
        Math::Vec3 axisZ = directionZ;
        axisZ.normalize();

        Math::Vec3 axisY = Math::cross(axisZ, axisX);
        axisY.normalize();

        Math::Mat4 rotate{axisX, axisY, axisZ, Math::Vec3::ZERO};
        Math::Mat4 translate = Math::translate(*startPos);

        midPointAnchorTransform = translate * rotate;
        lengthScaleX = directionX.length() - 0.25;
    }

    if (!midPointAnchorTransform || !messengerPos)
    {
        return;
    }

    Math::Vec3 insertPos = midPointAnchorTransform->getTranslate();
    double insertHeight = (*messengerPos - insertPos).length();

    std::optional<electrification::CatenaryAttachableLength> lengthType;
    std::string nodeName;
    if (m_genParams.midPointAnchorType == MidPointAnchorType::MIDDLE)
    {
        NodeLengthMap lengthMap = getNodeLengths(
            m_genParams.contextData.scompMidPointAnchorName);
        lengthType = chooseNodeLengthType(lengthMap, insertHeight);
        if (lengthType)
        {
            nodeName = m_genParams.contextData.scompMidPointAnchorName[*lengthType];
        }
    }
    else
    {
        NodeLengthMap lengthMap = getNodeLengths(
            m_genParams.contextData.compMidPointAnchorName);
        lengthType = chooseNodeLengthType(lengthMap, insertHeight);
        if (lengthType)
        {
            nodeName = m_genParams.contextData.compMidPointAnchorName[*lengthType];
        }
    }

    if (nodeName.empty())
    {
        // TODO: maybe log error.
        return;
    }

    NodePtr node = World::loadNode(nodeName.c_str());
    if (!node)
    {
        return;
    }

    WorldBoundBox boundBox = node->getWorldBoundBox();
    Math::Vec3 minPoint = node->getWorldRotation() * boundBox.getMin();
    Math::Vec3 maxPoint = node->getWorldRotation() * boundBox.getMax();

    minPoint = *midPointAnchorTransform * (minPoint);
    maxPoint = *midPointAnchorTransform * (maxPoint);

    double minParam = m_messengerWire->getParamByAbsolutePosition(minPoint);
    double maxParam = m_messengerWire->getParamByAbsolutePosition(maxPoint);

    std::optional<Math::Vec3> mPos = m_messengerWire->eval(maxParam);
    double anchorHeight = mPos->z - minPoint.z;
    double nodeLengthZ = maxPoint.z - minPoint.z;
    double lengthScaleZ = anchorHeight / nodeLengthZ;

    if (m_genParams.midPointAnchorType != MidPointAnchorType::MIDDLE)
    {
        double nodeLengthX = (maxPoint.xy - minPoint.xy).length();
        lengthScaleX = lengthScaleX / nodeLengthX;
    }

    Math::Mat4 scale = Math::scale(Math::Vec3(lengthScaleX, 1., lengthScaleZ));
    midPointAnchorTransform = *midPointAnchorTransform * scale;

    if (minParam > maxParam)
    {
        std::swap(minParam, maxParam);
    }

    AttachableInfo midPointAnchor;
    if (m_genParams.midPointAnchorType == MidPointAnchorType::MIDDLE)
    {
        midPointAnchor.type =
            electrification::CatenaryAttachableType::SemiCompensatedMidpointAnchor;
    }
    else
    {
        midPointAnchor.type =
            electrification::CatenaryAttachableType::CompensatedMidpointAnchor;
    }

    midPointAnchor.lengthType = *lengthType;
    midPointAnchor.transform = *midPointAnchorTransform;
    m_attachableTransforms.push_back(midPointAnchor);

    boost::icl::continuous_interval<double> interval{minParam, maxParam};
    m_attachablesIntervals.insert(interval);
}

void
DynamicElectrificationCatenaryGenerator::generateSpanWireConnector()
{
    if (!m_messengerWire || m_dropperParams.empty() || !m_genParams.isCrossSpan)
    {
        return;
    }

    double closestDropperParam = 0.5;
    double minDistance = std::numeric_limits<double>::max();

    for (double param : m_dropperParams)
    {
        double distance = std::abs(param - 0.5);
        if (distance < minDistance)
        {
            minDistance = distance;
            closestDropperParam = param;
        }
    }

    auto nearIt = m_contactWires.find(ELECTRIFICATION_POLE_WIRE_CONNECTOR_NEAR_INDEX);
    if (nearIt == m_contactWires.end())
    {
        return;
    }

    auto contactWire = nearIt->second;

    std::optional<Math::Vec3> insertPos = contactWire->eval(closestDropperParam);
    if (!insertPos)
    {
        return;
    }

    const Math::Vec3 spanWireOffset{0.0, m_spanConnectorOffset, 0.0};
    insertPos = *insertPos + spanWireOffset;

    const double insertParam = m_messengerWire->getParamByPositionOnVerticalLine(*insertPos);
    std::optional<Math::Vec3> messengerPos = m_messengerWire->eval(insertParam);
    if (!messengerPos)
    {
        return;
    }

    Math::Vec3 directionZ = *messengerPos - *insertPos;
    Math::Vec3 axisZ = directionZ;
    axisZ.normalize();

    Math::Vec3 axisY = Math::cross(axisZ, Math::Vec3::FORWARD);
    axisY.normalize();

    Math::Mat4 rotate{Math::Vec3::FORWARD, axisY, axisZ, Math::Vec3::ZERO};
    Math::Mat4 connectorTransform = Math::translate(*insertPos) * rotate;

    double insertHeight = directionZ.length();

    std::string nodeName;
    NodeLengthMap lengthMap = getNodeLengths(
        m_genParams.contextData.spanWireConnectorNodeName);

    std::optional<electrification::CatenaryAttachableLength> lengthType =
        chooseNodeLengthType(lengthMap, insertHeight);
    if (lengthType)
    {
        double nodeLength = lengthMap.at(*lengthType);
        double scaleHeight = insertHeight / nodeLength;
        Math::Mat4 scale = Math::scale(Math::Vec3(1., 1., scaleHeight));

        nodeName = m_genParams.contextData.spanWireConnectorNodeName[*lengthType];

        AttachableInfo spanWireConnector;
        spanWireConnector.type = electrification::CatenaryAttachableType::SpanWireConnector;
        spanWireConnector.lengthType = *lengthType;
        spanWireConnector.transform = connectorTransform * scale;

        m_attachableTransforms.push_back(spanWireConnector);
    }
}

void
DynamicElectrificationCatenaryGenerator::generateStraightWire(Math::Vec3 const& startPos,
    Math::Vec3 const& endPos, bool addConnector, std::string const& surfaceName)
{
    double knotHeight = m_contactRadius * 4;
    double knotRadius = m_contactRadius * 2;

    Math::Vec3 direction = endPos - startPos;
    double length = direction.length();

    Math::Vec3 segmentEndPos = Math::Vec3::RIGHT * length;

    Math::Vec3 rightTangent = segmentEndPos * 0.3;
    Math::Vec3 leftTangent = -rightTangent;

    BezierSegment segment(bezierSegmentByPositionsAndTangents(
        Math::Vec3::ZERO, segmentEndPos,
        rightTangent, leftTangent));

    Math::mat4 basis;
    if (std::abs(direction.x) < 1e-6 && std::abs(direction.y) < 1e-6)
    {
        basis = getVerticalTransform(startPos);
    }
    else
    {
        basis = Math::mat4(transformByTwoPoints(startPos, endPos));
    }
    basis = basis * Math::mat4(Math::rotateZ(90.));

    MeshPtr mesh = Mesh::create();

    Wire::generateWireMeshFromSpline(segment, m_dropperRadius, "wire", mesh);
    mesh->setSurfaceTransform(basis);

    addMeshPart(mesh, surfaceName);

    if (addConnector)
    {
        Math::Vec3 connectorPos = endPos + direction * (-ConnectorOffset);
        generateWireConnector(connectorPos, endPos, surfaceName + "_knot");
    }
}

void DynamicElectrificationCatenaryGenerator::addMeshPart(MeshPtr const& meshToAdd,
    std::string const& surfaceName)
{
    for (int i = 0; i < meshToAdd->getNumSurfaces(); i++)
    {
        std::string name = surfaceName + std::to_string(i);
        m_mesh->addMeshSurface(name.c_str(), meshToAdd, i);
    }
}

Math::mat4
DynamicElectrificationCatenaryGenerator::getVerticalTransform(Math::Vec3 point) const
{
    Math::Mat4 transform = globalTransform();
    Math::Mat4 newTransform{transform.getAxisX(), -transform.getAxisZ(), transform.getAxisY(),
        point};
    return Math::mat4(newTransform);
}

std::optional<Math::Vec3>
DynamicElectrificationCatenaryGenerator::getAverageContactWirePos(double tParam)
{
    if (!m_messengerWire)
    {
        return std::nullopt;
    }

    std::optional<Math::Vec3> messengerPos = m_messengerWire->eval(tParam);
    if (!messengerPos)
    {
        return std::nullopt;
    }

    std::optional<Math::Vec3> contactPos;
    int posFound = 0;
    for (auto& [connectorId, contactWire] : m_contactWires)
    {
        const double contactParam = contactWire->getParamByAbsolutePosition(*messengerPos);
        std::optional<Math::Vec3> wirePos = contactWire->eval(contactParam);
        if (!wirePos)
        {
            continue;
        }

        if (contactPos)
        {
            if (wirePos)
            {
                posFound += 1;
                contactPos = *contactPos + *wirePos;
            }
        }
        else
        {
            contactPos = wirePos;
        }
    }

    if (!contactPos)
    {
        return std::nullopt;
    }

    posFound += 1;
    contactPos = *contactPos / posFound;
    return contactPos;
}

double
DynamicElectrificationCatenaryGenerator::maxOffset() const
{
    return maxCableOffsetByCoord(
        m_genParams.contextData.messengerWire.startPosition,
        m_genParams.contextData.messengerWire.endPosition, m_saggingCoefficient,
        m_genParams.contextData.leftStitch || m_genParams.contextData.rightStitch);
}

std::vector<double>
DynamicElectrificationCatenaryGenerator::getDropperParams()
{
    auto calcNumDroppers = [this](double airLength) -> int
    {
        const std::map<double, int, std::greater<double>> numDroppersStitched = {
            {77.0, 7},
            {67.0, 6},
            {57.0, 5},
            {47.0, 4},
            {37.0, 3},
            {30.0, 2},
            {25.0, 1}
        };

        const std::map<double, int, std::greater<double>> numDroppersSimple = {
            {77.0, 9},
            {67.0, 8},
            {57.0, 7},
            {47.0, 6},
            {37.0, 5},
            {27.0, 4},
            {17.0, 3},
            { 7.0, 2},
            { 4.0, 1}
        };

        std::map<double, int, std::greater<double>> const* lengthToNumDroppers{nullptr};
        if (m_genParams.contextData.leftStitch || m_genParams.contextData.rightStitch)
        {
            lengthToNumDroppers = &numDroppersStitched;
        }
        else
        {
            lengthToNumDroppers = &numDroppersSimple;
        }

        for (auto [length, num] : *lengthToNumDroppers)
        {
            if (airLength > length)
            {
                if (!m_genParams.contextData.leftStitch || !m_genParams.contextData.rightStitch)
                {
                    return num;
                }
                else
                {
                    return num + 1;
                }
            }
        }

        return 0;
    };

    const double minDropperStep = 0.1;

    std::vector<double> result;
    if (!m_messengerWire)
    {
        return result;
    }

    const double airLength = m_messengerWire->airLength();

    if (airLength < ELECTRIFICATION_STITCHED_CATENARY_LENGTH_MIN)
    {
        m_genParams.contextData.leftStitch = false;
        m_genParams.contextData.rightStitch = false;
    }

    int numDroppers = calcNumDroppers(airLength);

    if (numDroppers == 0)
    {
        return result;
    }

    if (numDroppers == 1)
    {
        result.push_back(0.5);
        return result;
    }

    result.reserve(numDroppers);
    double firstDistance;
    if (m_genParams.contextData.leftStitch)
    {
        firstDistance = ELECTRIFICATION_FIRST_DROPPER_DISTANCE_STITCHED;
    }
    else
    {
        firstDistance = ELECTRIFICATION_FIRST_DROPPER_DISTANCE_SIMPLE;
    }

    double lastDistance;
    if (m_genParams.contextData.rightStitch)
    {
        lastDistance = airLength - ELECTRIFICATION_FIRST_DROPPER_DISTANCE_STITCHED;
    }
    else
    {
        lastDistance = airLength - ELECTRIFICATION_FIRST_DROPPER_DISTANCE_SIMPLE;
    }

    const double param0 = m_messengerWire->getParamByDistance(firstDistance);
    const double param1 = m_messengerWire->getParamByDistance(lastDistance);
    double precalculatedDistances = firstDistance * 2;

    if (m_genParams.midPointAnchorType == MidPointAnchorType::START ||
        m_genParams.midPointAnchorType == MidPointAnchorType::END)
    {
        numDroppers -= 1;
        precalculatedDistances += ELECTRIFICATION_COMP_MID_POINT_ANCH_STR_DISTANCE;
    }

    result.push_back(param0);

    if (numDroppers > 3)
    {
        double currentDistance = firstDistance;
        if (m_genParams.midPointAnchorType == MidPointAnchorType::END)
        {
            lastDistance -= ELECTRIFICATION_COMP_MID_POINT_ANCH_STR_DISTANCE;
        }

        if (m_genParams.midPointAnchorType == MidPointAnchorType::START)
        {
            currentDistance += ELECTRIFICATION_COMP_MID_POINT_ANCH_STR_DISTANCE;
            const double paramAnch = m_messengerWire->getParamByDistance(currentDistance);
            result.push_back(paramAnch);
        }

        const double dropperStep = (airLength - precalculatedDistances) / (numDroppers - 2 + 1);
        currentDistance += dropperStep;
        while (currentDistance < lastDistance - minDropperStep)
        {
            const double currentParam = m_messengerWire->getParamByDistance(currentDistance);
            result.push_back(currentParam);
            currentDistance += dropperStep;
        }

        if (m_genParams.midPointAnchorType == MidPointAnchorType::END)
        {
            currentDistance = lastDistance;
            const double paramAnch = m_messengerWire->getParamByDistance(currentDistance);
            result.push_back(paramAnch);
        }
    }
    else if (numDroppers == 3)
    {
        result.push_back(0.5);
    }

    result.push_back(param1);

    return result;
}

std::vector<double>
DynamicElectrificationCatenaryGenerator::getInsulatedOverlapDropperParams()
{
    std::vector<double> result;

    auto insulatorIt = m_insulatorsByType.find(InsulatorPositionType::OverlapLower);
    if (insulatorIt == m_insulatorsByType.end())
    {
        insulatorIt = m_insulatorsByType.find(InsulatorPositionType::OverlapUpper);
        if (insulatorIt == m_insulatorsByType.end())
        {
            return result;
        }
    }

    InsulatorPtr insulator = insulatorIt->second;
    assert(insulator);
    if (!insulator)
    {
        return result;
    }

    auto inPos = insulator->getConnectorPosition(AttachableConnectorType::KP_NEAR_IN);
    auto outPos = insulator->getConnectorPosition(AttachableConnectorType::KP_NEAR_OUT);

    if (!inPos || !outPos)
    {
        inPos = insulator->getConnectorPosition(AttachableConnectorType::NT_IN);
        outPos = insulator->getConnectorPosition(AttachableConnectorType::NT_OUT);

        if (!inPos || !outPos)
        {
            return result;
        }
    }

    inPos = insulator->getWorldTransform() * *inPos;
    outPos = insulator->getWorldTransform() * *outPos;

    for (int i = 1; i <= m_numInsulatorDroppers; i++)
    {
        Math::Vec3 inPosOffseted = *inPos -
            Math::Vec3::FORWARD * (m_insulatorDropperStepM * i);
        Math::Vec3 outPosOffseted = *outPos +
            Math::Vec3::FORWARD * (m_insulatorDropperStepM * i);

        double inParam = m_messengerWire->getParamByAbsolutePosition(inPosOffseted);
        double outParam = m_messengerWire->getParamByAbsolutePosition(outPosOffseted);

        result.push_back(inParam);
        result.push_back(outParam);
    }

    return result;
}

Math::Vec3
DynamicElectrificationCatenaryGenerator::getTranslate() const
{
    return m_genParams.contextData.messengerWire.startPosition;
}

Math::Mat4
DynamicElectrificationCatenaryGenerator::toLocalTransform() const
{
    Math::Mat4 transform0 = globalTransform();
    Math::Mat4 transform1 = transformByTwoPoints(Math::Vec3::ZERO,
        Math::Vec3::FORWARD);

    return Math::inverse(transform0) * transform1;
}

Math::Mat4
DynamicElectrificationCatenaryGenerator::globalTransform() const
{
    Math::Mat4 transform0 = transformByTwoPoints(
        m_genParams.contextData.messengerWire.startPosition,
        m_genParams.contextData.messengerWire.endPosition);

    return transform0;
}

Unigine::NodePtr DynamicElectrificationCatenaryGenerator::rootNode() const
{
    return m_rootNode;
}

DynamicElectrificationCatenaryGenerator::NodeLengthMap
DynamicElectrificationCatenaryGenerator::getNodeLengths(
    electrification::CatenaryAttachableNodeName const& nodeNames)
{
    NodeLengthMap result;
    for (auto& [sizeType, nodeName] : nodeNames)
    {
        NodePtr node = World::loadNode(nodeName.c_str());
        if (!node)
        {
            continue;
        }

        WorldBoundBox boundBox = node->getWorldBoundBox();
        node.deleteLater();

        Math::Vec3 minPoint = boundBox.getMin();
        Math::Vec3 maxPoint = boundBox.getMax();

        double height = std::abs(maxPoint.z - minPoint.z);
        double widthX = std::abs(maxPoint.x - minPoint.x);
        if (widthX > m_maxDropperWidth)
        {
            m_maxDropperWidth = widthX;
        }

        result.emplace(sizeType, height);
    }

    return result;
}

std::optional<electrification::CatenaryAttachableLength>
DynamicElectrificationCatenaryGenerator::chooseNodeLengthType(NodeLengthMap const& nodeLengths,
    double distance)
{
    if (nodeLengths.empty())
    {
        return std::nullopt;
    }

    NodeLengthMap differenceMap;
    for (auto& [sizeType, size] : nodeLengths)
    {
        double difference = std::abs(size - distance);
        differenceMap.emplace(sizeType, difference);
    }

    auto minDifference = std::min_element(
        differenceMap.begin(), differenceMap.end(),
        [](const auto& l, const auto& r) { return l.second < r.second; });

    if (minDifference->second > ELECTRIFICATION_DROPPER_LENGTH_MAX)
    {
        return std::nullopt;
    }

    return minDifference->first;
}

void
DynamicElectrificationCatenaryGenerator::addNodes(
    std::function<void(std::string const&, Unigine::Math::Mat4 const&)> &addNodeFunc) const
{
    // insulators
    if (m_messengerWire)
    {
        auto& insulators = m_messengerWire->getMorticeAttachables();
        for (auto const& [param, insulator] : insulators)
        {
            Math::Mat4 insulatorTransform;

            if (m_genParams.creationMode != electrification::CreationMode::CLUSTER)
            {
                insulatorTransform = insulator->getWorldTransform();
            }
            else
            {
                // For clusters, use transform without the initial rotation because
                // it's applied for clusters automatically.
                insulatorTransform = insulator->getWorldWithNoInitialTransform();
            }

            addNodeFunc(insulator->nodeName(), insulatorTransform);
        }
    }

    for (auto const& [connectorId, contactWire] : m_contactWires)
    {
        if (connectorId != ELECTRIFICATION_POLE_WIRE_CONNECTOR_NEAR_INDEX)
        {
            continue;
        }

        auto& insulators = contactWire->getMorticeAttachables();
        for (auto const& [param, insulator] : insulators)
        {
            addNodeFunc(insulator->nodeName(), insulator->getWorldTransform());
        }
    }

    auto tryAddDropperNode = [this, &addNodeFunc](
        CatenaryAttachableNodeName const* collection,
        AttachableInfo const& info)
    {
        auto findIt = collection->find(info.lengthType);
        if (findIt != collection->cend())
        {
            addNodeFunc(findIt->second, info.transform);
        }
        else
        {
            warning("DynamicElectrificationCatenaryGenerator::addNodes\n");
            warning("Can not add dropper node");
        }
    };

    std::unordered_map<CatenaryAttachableType, CatenaryAttachableNodeName const*> dropperNodeListsByType
    {
         { CatenaryAttachableType::Dropper,
         &m_genParams.contextData.dropperNodeName },
         { CatenaryAttachableType::Dropper1w,
         &m_genParams.contextData.dropperNodeName1w },
         { CatenaryAttachableType::InsulatorDropper,
         &m_genParams.contextData.insulatorDropperNodeName },
         { CatenaryAttachableType::InsulatorDropper1w,
         &m_genParams.contextData.insulatorDropperNodeName1w },
         { CatenaryAttachableType::CompensatedMidpointAnchor,
         &m_genParams.contextData.compMidPointAnchorName },
         { CatenaryAttachableType::SemiCompensatedMidpointAnchor,
         &m_genParams.contextData.scompMidPointAnchorName }
    };

    // attachables
    for (auto const& attachableInfo : m_attachableTransforms)
    {
        auto const* dropperNodeCollection = dropperNodeListsByType[attachableInfo.type];
        tryAddDropperNode(dropperNodeCollection, attachableInfo);
        if (attachableInfo.type == electrification::CatenaryAttachableType::SpanWireConnector)
        {
            addNodeFunc(
                m_genParams.contextData.spanWireConnectorNodeName.at(attachableInfo.lengthType),
                attachableInfo.transform);
        }
    }
}

void
DynamicElectrificationCatenaryGenerator::addDropper(int dropperId, Math::Vec3 const& messengerPos,
    Math::Vec3 const& contactPos, bool force1w,
    DynamicElectrificationCatenaryGenerator::NodeLengthMap const& lengthMap)
{
    const bool doAddConnectors = false;
    auto generateMeshDropper = [this, doAddConnectors](
        int dropperId, Math::Vec3 messengerPos, Math::Vec3 contactPos)
    {
        std::string surfaceName = "dropper_" + std::to_string(dropperId);
        generateStraightWire(messengerPos, contactPos, false, surfaceName);

        Math::Vec3 connectorPos = messengerPos + (contactPos - messengerPos) * ConnectorOffset;
        if (doAddConnectors)
        {
            generateWireConnector(connectorPos, messengerPos,
                "dropper_connector_main" + std::to_string(dropperId));
        }

        for (auto& [wireId, wire] : m_contactWires)
        {
            if (wireId == m_mainWireId)
            {
                continue;
            }

            const double wireParam = wire->getParamByPositionOnVerticalLine(contactPos);
            std::optional<Math::Vec3> wirePos = wire->eval(wireParam);
            if (!wirePos)
            {
                continue;
            }

            double distance = (*wirePos - contactPos).length();
            if (distance > ELECTRIFICATION_DROPPER_LENGTH_MAX)
            {
                continue;
            }

            if (doAddConnectors)
            {
                generateWireConnector(contactPos, *wirePos,
                    "dropper_connector_" + std::to_string(wireId));
            }
        }
    };

    Math::Vec3 messengerPosUpdated = messengerPos;

    Math::Vec3 directionZ = messengerPosUpdated - contactPos;
    double dropperHeight = directionZ.length();
    if (dropperHeight > ELECTRIFICATION_DROPPER_LENGTH_MAX)
    {
        const double newParam = m_messengerWire->getParamByPositionOnVerticalLine(contactPos);
        if (auto newPos = m_messengerWire->eval(newParam))
        {
            messengerPosUpdated = *newPos;
            directionZ = messengerPosUpdated - contactPos;
            dropperHeight = directionZ.length();
        }
    }

    std::optional<electrification::CatenaryAttachableLength> lengthType =
        chooseNodeLengthType(lengthMap, dropperHeight);
    if (lengthType)
    {
        double nodeLength = lengthMap.at(*lengthType);

        Math::Vec3 axisZ = directionZ;
        axisZ.normalize();

        Math::Vec3 axisY = Math::cross(axisZ, Math::Vec3::FORWARD);
        axisY.normalize();

        Math::Mat4 rotate{Math::Vec3::FORWARD, axisY, axisZ, Math::Vec3::ZERO};

        double scaleHeight = dropperHeight / nodeLength;
        Math::Mat4 scale = Math::scale(Math::Vec3(1., 1., scaleHeight));
        Math::Mat4 translate = Math::translate(contactPos);

        AttachableInfo dropperInfo;
        if (force1w)
        {
            dropperInfo.type = electrification::CatenaryAttachableType::Dropper1w;
        }
        else
        {
            dropperInfo.type = electrification::CatenaryAttachableType::Dropper;
        }
        dropperInfo.lengthType = *lengthType;
        dropperInfo.transform = translate * rotate * scale;

        m_attachableTransforms.push_back(dropperInfo);
    }
    else
    {
        generateMeshDropper(dropperId, messengerPos, contactPos);
    }
}

void
DynamicElectrificationCatenaryGenerator::maybeCreateRootNode()
{
    if (!m_rootNode)
    {
        String nodeName;
        if (m_genParams.isSpanWire)
        {
            nodeName = String::format("reg_wire_segment_%d_%d", m_key.id, m_key.subId);
        }
        else
        {
            nodeName = String::format("catenary_segment_%d_%d", m_key.id, m_key.subId);
        }

        Math::Vec3 rootPosition = getBoundBox().getCenter();

        m_rootNode = NodeDummy::create();
        m_rootNode->setName(nodeName);
        m_rootNode->setWorldTransform(globalTransform());
    }
}
