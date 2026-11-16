#include "ElectrificationAltNodeNameChooser.h"

#include "neokon-shared/FloatingPointCompare.h"
#include "neokon-shared/StringFormat.h"

#include <UnigineEngine.h>

#include <filesystem>
#include <mutex>
#include <sstream>

using namespace electrification;

namespace
{

const std::unordered_map<RegistrationArmType, std::string> s_registrationArmTypes
{
    { RegistrationArmType::StraightPullOff, "pz" },
    { RegistrationArmType::StraightPushOff, "oz" },
    { RegistrationArmType::CurvePullOff, "pcv" },
    { RegistrationArmType::CurvePushOff, "ocv" },
    { RegistrationArmType::StraightJunction, "jz" },
    { RegistrationArmType::CurveJunction, "jcv" }
};

std::set<std::string> existingRelativeFilePaths;
std::mutex existingPathsMutex;

/**
 * @p relativePath is path relative to GTE/data.
 *
 * TODO: implement synchronization without locking.
 */
bool
fileExists(std::string const& relativePath)
{
    std::lock_guard<std::mutex> lock(existingPathsMutex);

    if (existingRelativeFilePaths.count(relativePath) > 0)
    {
        return true;
    }

    std::filesystem::path rootPath = Unigine::Engine::get()->getDataPath();
    std::filesystem::path resultPath = rootPath / relativePath;

    bool const exists = std::filesystem::exists(resultPath);
    if (exists)
    {
        existingRelativeFilePaths.insert(relativePath);
    }

    return exists;
}

std::string makePolePrefix(ElectrificationMeshGeneratorConfigPtr const& config,
                           PoleContextData const& contextData)
{
    std::stringstream ss;

    ss << config->getCommonParamsData().contactWireTypeStr;
    ss << '_';

    auto findIt = s_registrationArmTypes.find(contextData.registrationArmType);
    assert(findIt != s_registrationArmTypes.cend());

    ss << findIt->second;

    return ss.str();
}

PoleContextData
replaceArmCurveToStraight(PoleContextData const& contextData)
{
    PoleContextData result = contextData;

    if (contextData.registrationArmType == RegistrationArmType::CurvePullOff)
    {
        result.registrationArmType = RegistrationArmType::StraightPullOff;
    }
    else if (contextData.registrationArmType == RegistrationArmType::CurvePushOff)
    {
        result.registrationArmType = RegistrationArmType::StraightPushOff;
    }

    return result;
}

std::string replaceAsteriskByPrefix(std::string const& srcString, std::string const& prefix)
{
    std::size_t pos = srcString.find('*', 0);
    if (pos == std::string::npos)
    {
        return srcString;
    }

    std::size_t lastPos = srcString.size() - 1;

    std::stringstream ss;
    ss << srcString.substr(0, pos);
    ss << prefix;

    if (pos < lastPos)
    {
        ss << srcString.substr(pos + 1);
    }

    return ss.str();
}

std::string checkCurveRegistrationArmExistence(std::string const& sourcePath,
    PoleContextData const& contextData,
    ElectrificationMeshGeneratorConfigPtr const& config)
{
    std::string prefix = makePolePrefix(config, contextData);
    std::filesystem::path relativePath = replaceAsteriskByPrefix(sourcePath, prefix);
    /**
     * Относительный путь, указанный в конфиге иногда может содержать начальный
     * прямой слеш /, который библиотека std::filesystem воспринимает как корневой
     * каталог Linux. Мы должны отбросить его перед объединением корневого и относительного
     * пути в абсолютный путь.
     */
    relativePath = relativePath.relative_path();

    // TODO: perform test for correct node name with no file existance testing.
    if (!fileExists(relativePath.string()))
    {
        /*
         * Некоторые наборы моделей опор (или подвесов под жесткие поперечины)
         * могут не включать в себя отдельные модели с фиксаторами для закругленных участков пути.
         * Они имеют префиксы в названии файлов, такие как pcv или ocv. Если таковых в ассетах
         * не обнаружено, то мы заменяем их аналогами для прямых участков пути с префиксами pz и oz.
         */

        auto alteredContextData = replaceArmCurveToStraight(contextData);
        prefix = makePolePrefix(config, alteredContextData);
        relativePath = replaceAsteriskByPrefix(sourcePath, prefix);

        relativePath = relativePath.relative_path();
    }

    return relativePath.string();
}

std::optional<int> scanLengthPrefixMM(std::string const& srcString)
{
    enum class eState
    {
        Scanning,
        LengthPrefix,
        Digits,
        Finish
    };

    eState currentState{ eState::Scanning };
    int startPosition{ -1 };
    int endPosition{ -1 };

    for (int i = 0; i < srcString.size(); i++)
    {
        char currentChar = srcString[i];

        switch (currentState)
        {
        case eState::Scanning:
        {
            if (currentChar == '_')
            {
                currentState = eState::LengthPrefix;
            }
            break;
        }
        case eState::LengthPrefix:
        {
            if (currentChar == 'L')
            {
                currentState = eState::Digits;
            }
            else
            {
                currentState = eState::Scanning;
            }

            break;
        }
        case eState::Digits:
        {
            if (isdigit(currentChar))
            {
                if (startPosition < 0)
                {
                    startPosition = i;
                }
                endPosition = i;
            }
            else
            {
                endPosition++;
                currentState = eState::Finish;
            }

            break;
        }
        }//switch (currentState)

        if (currentState == eState::Finish)
        {
            break; //for (int i = 0; i < srcString.size(); i++)
        }
    }//for (int i = 0; i < srcString.size(); i++)

    if (startPosition >= 0 && endPosition > startPosition)
    {
        std::string digitsString = srcString.substr(startPosition, endPosition - startPosition);

        int result = atoi(digitsString.c_str());
        if (!result)
        {
            Unigine::Log::error("Can not convert string value \"%s\" to numeric value.\n", digitsString.c_str());

            return std::nullopt;
        }

        return result;
    }

    return std::nullopt;
}

std::vector<int> scanDirectoryForLengths(std::string const& srcPathStr, std::string const& rootDirectoryStr)
{
    namespace fs = std::filesystem;

    fs::path relativePath = srcPathStr;
    relativePath = relativePath.parent_path().relative_path();

    fs::path absolutePath = rootDirectoryStr;
    absolutePath /= relativePath;
    absolutePath = absolutePath.make_preferred();

    std::unordered_set<int> lengthsSet;

    for (auto const& dirEntry : fs::directory_iterator{ absolutePath })
    {
        auto length = scanLengthPrefixMM(dirEntry.path().filename().string());

        if (!length || lengthsSet.count(*length) > 0)
        {
            continue;
        }

        lengthsSet.insert(*length);
    }

    std::vector<int> result;

    if (!lengthsSet.empty())
    {
        result.insert(result.begin(), lengthsSet.cbegin(), lengthsSet.cend());
        std::sort(result.begin(), result.end());
    }

    return result;
}

std::optional<int>
findSuitableObjectLength(int desiredLengthMM, std::vector<int> const& availableLengths)
{
    if (availableLengths.empty())
    {
        return std::nullopt;
    }

    if (desiredLengthMM < availableLengths.front())
    {
        return availableLengths.front();
    }

    if (desiredLengthMM > availableLengths.back())
    {
        return availableLengths.back();
    }

    for (int i = 0; i < availableLengths.size(); i++)
    {
        if (availableLengths[i] >= desiredLengthMM)
        {
            return availableLengths[i];
        }
    }

    return std::nullopt;
}

std::string makeRigelLengthPrefix(int length)
{
    return "L" + std::to_string(length) + "mm_";
}

int metersToMillimeters(double value)
{
    return static_cast<int>(value * 1000.0);
}

double millimetersToMeters(int value)
{
    return static_cast<double>(value) * 0.001;
}

} // anonymous namespace

ElectrificationAltNodeNameChooser::ElectrificationAltNodeNameChooser(
    ElectrificationMeshGeneratorConfigPtr const& config)
    : m_config(config)
{
    std::string rootPath = Unigine::Engine::get()->getDataPath();
    m_cachedRigelAvailableLengths = scanDirectoryForLengths(config->getPortalData().crossBarNode, rootPath);
}

std::optional<std::string>
ElectrificationAltNodeNameChooser::choosePoleNodeName(
    PoleContextData const& contextData) const
{
    if (contextData.nodeName)
    {
        auto customNodeNames = m_config->getCustomNodeData();
        auto searchResult =
            customNodeNames.nodePathsByNames.find(*contextData.nodeName);
        if (searchResult != customNodeNames.nodePathsByNames.end())
        {
            return searchResult->second;
        }
    }

    std::optional<std::string> path;

    if (contextData.poleType == PoleType::Transitional)
    {
        assert(contextData.transitionalPoleSubtype);

        auto const& doublePoleNode = m_config->getPoleData().poleDoubleNode;

        std::optional<std::string> nodeName;
        switch (*contextData.transitionalPoleSubtype)
        {
        case TransitionalPoleType::TransitionalA:
            nodeName = doublePoleNode.poleA;
            break;
        case TransitionalPoleType::TransitionalB:
            nodeName = doublePoleNode.poleB;
            break;
        case TransitionalPoleType::TransitionalC:
            nodeName = doublePoleNode.poleC;
            break;
        case TransitionalPoleType::TransitionalAInsulated:
            nodeName = doublePoleNode.poleInsA;
            break;
        case TransitionalPoleType::TransitionalBInsulated:
            nodeName = doublePoleNode.poleInsB;
            break;
        case TransitionalPoleType::TransitionalCInsulated:
            nodeName = doublePoleNode.poleInsC;
            break;
        }

        if (nodeName)
        {
            std::filesystem::path fsPath = doublePoleNode.poleDoubleNodePath;
            fsPath /= *nodeName;

            path = fsPath.string();
        }
    }
    else
    {
        path = m_config->getPoleData().poleNode;
    }

    if (path)
    {
        return checkCurveRegistrationArmExistence(*path, contextData, m_config);
    }

    return path;
}

std::optional<std::string>
ElectrificationAltNodeNameChooser::choosePortalSuspensionNodeName(
    PoleContextData const& contextData) const
{
    if (contextData.nodeName)
    {
        auto customNodeNames = m_config->getCustomNodeData();
        auto searchResult = customNodeNames.nodePathsByNames.find(*contextData.nodeName);
        if (searchResult != customNodeNames.nodePathsByNames.end())
        {
            return searchResult->second;
        }
    }

    std::optional<std::string> path;

    if (contextData.poleType == PoleType::Transitional)
    {
        assert(contextData.transitionalPoleSubtype);

        auto const& spnNode = m_config->getPortalSuspensionData().spnDoubleNode;

        std::optional<std::string> nodeName;
        switch (*contextData.transitionalPoleSubtype)
        {
        case TransitionalPoleType::TransitionalA:
            nodeName = spnNode.spnA;
            break;
        case TransitionalPoleType::TransitionalB:
            nodeName = spnNode.spnB;
            break;
        case TransitionalPoleType::TransitionalC:
            nodeName = spnNode.spnC;
            break;
        case TransitionalPoleType::TransitionalAInsulated:
            nodeName = spnNode.spnInsA;
            break;
        case TransitionalPoleType::TransitionalBInsulated:
            nodeName = spnNode.spnInsB;
            break;
        case TransitionalPoleType::TransitionalCInsulated:
            nodeName = spnNode.spnInsC;
            break;
        }

        if (nodeName)
        {
            std::filesystem::path fsPath = spnNode.spnDoubleNodePath;
            fsPath /= *nodeName;

            path = fsPath.string();
        }
    }
    else
    {
        path = m_config->getPortalSuspensionData().spnNode;
    }

    if (path)
    {
        return checkCurveRegistrationArmExistence(*path, contextData, m_config);
    }

    return path;
}

std::optional<std::string>
ElectrificationAltNodeNameChooser::chooseAnchorBackTieNodeName(
    AnchorContextData const& contextData) const
{
    std::optional<std::string> path;

    if (!contextData.middleTensioning)
    {
        switch (contextData.tensioningType)
        {
        case TensioningType::Fixed:
            path = m_config->getAnchorData().fixedBackTieNode;
            break;
        case TensioningType::Compensated:
            path = m_config->getAnchorData().compensatedBackTieNode;
            break;
        case TensioningType::SemiCompensated:
            path = m_config->getAnchorData().semiCompensatedBackTieNode;
            break;
        }
    }
    else
    {
        path = m_config->getAnchorData().midBackTie;
    }

    if (path)
    {
        std::string prefix = m_config->getCommonParamsData().contactWireTypeStr;
        path = replaceAsteriskByPrefix(*path, prefix);
    }

    return path;
}

std::optional<std::string>
ElectrificationAltNodeNameChooser::chooseAnchorBlockNodeName(
    AnchorContextData const& contextData) const
{
    std::optional<std::string> path;

    if (!contextData.middleTensioning)
    {
        switch (contextData.tensioningType)
        {
        case TensioningType::Fixed:
            path = m_config->getAnchorData().fixedAnchorBlockNode;
            break;
        case TensioningType::Compensated:
            path = m_config->getAnchorData().compensatedAnchorBlockNode;
            break;
        case TensioningType::SemiCompensated:
            path = m_config->getAnchorData().semiCompensatedAnchorBlockNode;
            break;
        }
    }
    else
    {
        path = m_config->getAnchorData().midAnchorNode;
    }

    if (path)
    {
        std::string prefix = m_config->getCommonParamsData().contactWireTypeStr;
        path = replaceAsteriskByPrefix(*path, prefix);
    }

    return path;
}

std::optional<std::string>
ElectrificationAltNodeNameChooser::chooseCatenaryAttachableNodeName(
    CatenaryAttachableType attachableType,
    CatenaryAttachableLength attachableLengthType) const
{
    ElectrificationAltMeshGeneratorConfig::CatenaryAttachableNode attachable;

    bool forceDropper1w{ false };

    switch (attachableType)
    {
    case CatenaryAttachableType::Dropper:
        attachable = m_config->getCatenaryData().dropperNode;
        break;
    case CatenaryAttachableType::Dropper1w:
        attachable = m_config->getCatenaryData().dropperNode;
        forceDropper1w = true;
        break;
    case CatenaryAttachableType::InsulatorDropper:
        attachable = m_config->getCatenaryData().insulatorDropperNode;
        break;
    case CatenaryAttachableType::InsulatorDropper1w:
        attachable = m_config->getCatenaryData().insulatorDropperNode;
        forceDropper1w = true;
        break;
    case CatenaryAttachableType::CompensatedMidpointAnchor:
        attachable = m_config->getCatenaryData().compMidPointAnchorNode;
        break;
    case CatenaryAttachableType::SemiCompensatedMidpointAnchor:
        attachable = m_config->getCatenaryData().scompMidPointAnchorNode;
        break;
    case CatenaryAttachableType::SpanWireConnector:
        attachable = m_config->getPortalData().spanWireConnector;
        break;
    }

    std::optional<std::string> path;
    switch (attachableLengthType)
    {
    case CatenaryAttachableLength::Short:
        path = attachable.shortNode;
        break;
    case CatenaryAttachableLength::Medium:
        path = attachable.mediumNode;
        break;
    case CatenaryAttachableLength::Long:
        path = attachable.longNode;
        break;
    }

    if (path)
    {
        if (forceDropper1w)
        {
            std::string prefix = m_config->getCommonParamsData().additionalContactWireTypeStr;
            path = replaceAsteriskByPrefix(*path, prefix);
        }
        else
        {
            std::string prefix = m_config->getCommonParamsData().contactWireTypeStr;
            path = replaceAsteriskByPrefix(*path, prefix);
        }
    }

    return path;
}

std::optional<std::string>
ElectrificationAltNodeNameChooser::chooseLeftPortalPoleNodeName(
    PortalContextData const& contextData) const
{
    return m_config->getPortalData().leftPoleNode;
}

std::optional<std::string>
ElectrificationAltNodeNameChooser::chooseRightPortalPoleNodeName(
    PortalContextData const& contextData) const
{
    return m_config->getPortalData().rightPoleNode;
}

std::optional<PortalPartChoosingResult>
ElectrificationAltNodeNameChooser::choosePortalCrossbarNodeName(
    PortalContextData const& contextData) const
{
    const int desiredLengthMM = metersToMillimeters(contextData.desiredLength);

    if (auto matchedLength = findSuitableObjectLength(desiredLengthMM, m_cachedRigelAvailableLengths))
    {
        PortalPartChoosingResult result;

        std::string prefix = makeRigelLengthPrefix(*matchedLength);
        std::string srcPath = m_config->getPortalData().crossBarNode;

        result.path = replaceAsteriskByPrefix(srcPath, prefix);
        result.actualLength = millimetersToMeters(*matchedLength);

        return result;
    }

    return std::nullopt;
}
