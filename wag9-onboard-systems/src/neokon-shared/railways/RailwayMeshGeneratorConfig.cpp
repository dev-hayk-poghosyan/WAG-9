#include "RailwayMeshGeneratorConfig.h"

#include "cdt/CDT.h"
#include "neokon-shared/JsonStreamToProtoMessage.h"
#include "neokon-shared/ProtoMessageToJsonStream.h"
#include "neokon-shared/ProtobufUnigineInterop.h"

#include <UnigineEngine.h>
#include <UnigineMaterials.h>
#include <UnigineRender.h>
#include <UnigineDir.h>

#include <iostream>
#include <filesystem>
#include <fstream>
#include <regex>

using Triangulation = CDT::Triangulation<double>;

#define LOAD_IF_HAS(source, name, destination) if (source.has_##name##()) \
                                                    destination = source.##name##();

#define ASSIGN_IF_HAS(source, name, destination) if (source.has_##name##()) \
                                                      assign(destination, source.##name##());

using namespace Unigine;

const double RailwayMeshGeneratorConfig::toInnerUnitsScale = 0.001; //Millimeters -> Meters
const double RailwayMeshGeneratorConfig::toConfigUnitsScale = 1000.0; //Meters -> Millimeters

namespace
{

int
chooseSuitableLength(int availableSpaceMM, const std::vector<int>& arrowRailsLengths)
{
    int lastIdx = arrowRailsLengths.size() - 1;
    int result = arrowRailsLengths[lastIdx];

    for (size_t i = 0, numItems = arrowRailsLengths.size(); i < numItems; i++)
    {
        int lengthMM = arrowRailsLengths[i];
        if (lengthMM > availableSpaceMM)
        {
            continue;
        }

        result = lengthMM;
        break;
    }

    return result;
}

}//anonymous namespace

void
RailwayMeshGeneratorConfig::MaterialInfo::loadFromProto(const proto::meshgen::MaterialInfo& source)
{
    if (source.has_name())
    {
        name = source.name();
    }

    for (const auto& state : source.states())
    {
        states.emplace_back(MaterialParam{state.name(), state.value()});
    }

    for (const auto& texture : source.textures())
    {
        textures.emplace_back(MaterialParam{texture.name(), texture.value()});
    }

    for (const auto& param : source.params())
    {
        params.emplace_back(MaterialParam{param.name(), param.value()});
    }
}

void
RailwayMeshGeneratorConfig::MaterialInfo::saveToProto(proto::meshgen::MaterialInfo& source) const
{
    if (!name.empty())
    {
        source.set_name(name);
    }

    for (const auto& state : states)
    {
        auto protoState = source.add_states();
        *protoState->mutable_name() = state.name;
        *protoState->mutable_value() = state.value;
    }

    for (const auto& texture : textures)
    {
        auto protoTexture = source.add_textures();
        *protoTexture->mutable_name() = texture.name;
        *protoTexture->mutable_value() = texture.value;
    }

    for (const auto& param : params)
    {
        auto protoParam = source.add_textures();
        *protoParam->mutable_name() = param.name;
        *protoParam->mutable_value() = param.value;
    }
}

bool
RailwayMeshGeneratorConfig::MaterialInfo::hasParamsToApply() const
{
    return !states.empty() || !textures.empty() || !params.empty();
}

String
RailwayMeshGeneratorConfig::MaterialInfo::generateMatNameFromParams() const
{
    Unigine::String matName;

    auto addParams = [&matName](const std::vector<MaterialParam>& params)
    {
        for (const auto& param : params)
        {
            if (!matName.empty())
            {
                matName += "_";
            }
            matName += param.name.data();
            matName += "_";
            matName += param.value.data();
        }
    };
   
    addParams(states);
    addParams(textures);
    addParams(params);

    matName.replace("/", "_");

    return matName;
}

void
RailwayMeshGeneratorConfig::MaterialInfo::apply(const Unigine::MaterialPtr& mat) const
{
    for (const auto& state : states)
    {
        int stateIdx = mat->findState(state.name.data());
        if (stateIdx < 0)
        {
            Log::message("MaterialInfo::apply: Unknown state %s\n", state.name.data());
            continue;
        }
        mat->setState(stateIdx, std::stoi(state.value));
    }
    for (const auto& param : params)
    {
        int paramIdx = mat->findState(param.name.data());
        if (paramIdx < 0)
        {
            Log::message("MaterialInfo::apply: Unknown param %s\n", param.name.data());
            continue;
        }
        int paramaType = mat->getParameterType(paramIdx);
        switch (paramaType)
        {
        case Material::PARAMETER_FLOAT:
            mat->setParameterFloat(paramIdx, std::stof(param.value));
            break;
        case Material::PARAMETER_INT:
            mat->setParameterFloat(paramIdx, std::stof(param.value));
            break;
        default:
            Log::message("MaterialInfo::apply: Unsupported param type %d\n", paramaType);
            break;
        }
    }

    applyTextures(mat);
}

void
RailwayMeshGeneratorConfig::MaterialInfo::applyTextures(const Unigine::MaterialPtr& mat,
    const std::string& texPrefix) const
{
    for (const auto& texture : textures)
    {
        std::string texName(texPrefix);
        texName += texture.name;
        int texIdx = mat->findTexture(texName.data());
        if (texIdx < 0)
        {
            // Debugging message:
            // Log::message("MaterialInfo::apply: %s has no texture %s\n", mat->getParent()->getName(),
            //    texName.data());

            continue;
        }
        mat->setTexturePath(texIdx, texture.value.data());
    }
}

bool
RailwayMeshGeneratorConfig::MaterialInfo::operator==(const MaterialInfo& other) const
{
    if (name != other.name)
    {
        return false;
    }
    auto checkParams = [](const std::vector<MaterialParam>& firstParams,
                           const std::vector<MaterialParam>& secondParams)
    {
        for (const auto& first : firstParams)
        {
            auto secondMatchIt = std::find_if(secondParams.begin(), secondParams.end(),
                [&first](const MaterialParam& second)
                {
                    if (second.name == first.name)
                    {
                        if (second.value == first.value)
                        {
                            return true;
                        }
                    }
                    return false;
                });
            if (secondMatchIt == secondParams.end())
            {
                return false;
            }
        }
        return true;
    };
    if (!checkParams(states, other.states))
    {
        return false;
    }
    if (!checkParams(textures, other.textures))
    {
        return false;
    }
    if (!checkParams(params, other.params))
    {
        return false;
    }
    return true;
}

Unigine::MaterialPtr
RailwayMeshGeneratorConfig::MaterialInfo::getOrCreateMaterial(
    const MaterialInfo& addParams) const
{
    String matName(name.data());
    if (!addParams.name.empty())
    {
        matName = addParams.name.data();
    }
    auto baseMat = Materials::findMaterial(matName);
    if (!baseMat)
    {
        return {};
    }

    if (!hasParamsToApply() && !addParams.hasParamsToApply())
    {
        return baseMat;
    }

    matName += generateMatNameFromParams();
    matName += addParams.generateMatNameFromParams();

    auto tmpMaterial = Materials::findMaterial(matName);
    if (!tmpMaterial)
    {
        tmpMaterial = baseMat->inherit(matName);

        apply(tmpMaterial);
        addParams.apply(tmpMaterial);
    }
    return tmpMaterial;
}

//

RailwayMeshGeneratorConfig::RailwayMeshGeneratorConfig()
{
    for (int i = 0; i < +GeneratedSurfaceIdx::Total; ++i)
    {
        m_surfaceGenData.push_back(std::make_unique<SurfaceGenerationData>());
    }
}

bool
RailwayMeshGeneratorConfig::loadFromProto(const proto::meshgen::Config& meshGenProto)
{
    // Have to be first for override support
    for (const auto& include : meshGenProto.includes())
    {
        fillPropertiesFromJsonFile(Unigine::Engine::get()->getDataPath() + include);
    }

    for (const auto& pointProfileProto : meshGenProto.point_profiles())
    {
        m_pointProfiles.emplace_back(new PointProfileData);
        m_pointProfiles.back()->loadFromProto(pointProfileProto);
    }

    for (const auto& namedSurfaceProto : meshGenProto.named_surfaces())
    {
        m_namedSurfaces.emplace_back(new SurfaceGenerationData);
        m_namedSurfaces.back()->loadFromProto(namedSurfaceProto, m_pointProfiles);
    }

    if (meshGenProto.has_common_params())
    {
        m_commonGenData.loadFromProto(meshGenProto.common_params());
    }
    if (meshGenProto.has_branch_common())
    {
        m_branchCommonGenData.loadFromProto(meshGenProto.branch_common());
    }
    if (meshGenProto.has_junction_sleepers())
    {
        m_junctionSleepersGenData.loadFromProto(meshGenProto.junction_sleepers());
    }
    if (meshGenProto.has_branch_sleepers())
    {
        m_branchSleepersGenData.loadFromProto(meshGenProto.branch_sleepers());
    }
    if (meshGenProto.has_point_rails())
    {
        m_pointRailsGenData.loadFromProto(meshGenProto.point_rails());
    }
    if (meshGenProto.has_switch_rods())
    {
        m_switchRodsGenData[+SwitchRodsType::Default].loadFromProto(meshGenProto.switch_rods());
    }
    if (meshGenProto.has_switch_rods_x_front())
    {
        m_switchRodsGenData[+SwitchRodsType::XFront].loadFromProto(
            meshGenProto.switch_rods_x_front());
    }
    if (meshGenProto.has_switch_rods_x_back())
    {
        m_switchRodsGenData[+SwitchRodsType::XBack].loadFromProto(
            meshGenProto.switch_rods_x_back());
    }
    if (meshGenProto.has_switch_rods_x_front_left())
    {
        m_switchRodsGenData[+SwitchRodsType::XFrontLeft].loadFromProto(
            meshGenProto.switch_rods_x_front_left());
    }
    if (meshGenProto.has_switch_rods_x_front_right())
    {
        m_switchRodsGenData[+SwitchRodsType::XFrontRight].loadFromProto(
            meshGenProto.switch_rods_x_front_right());
    }
    if (meshGenProto.has_switch_rods_x_back_left())
    {
        m_switchRodsGenData[+SwitchRodsType::XBackLeft].loadFromProto(
            meshGenProto.switch_rods_x_back_left());
    }
    if (meshGenProto.has_switch_rods_x_back_right())
    {
        m_switchRodsGenData[+SwitchRodsType::XBackRight].loadFromProto(
            meshGenProto.switch_rods_x_back_right());
    }
    if (meshGenProto.has_switch_rods_three_way_first())
    {
        m_switchRodsGenData[+SwitchRodsType::TWFirst].loadFromProto(
            meshGenProto.switch_rods_three_way_first());
    }
    if (meshGenProto.has_switch_rods_three_way_second())
    {
        m_switchRodsGenData[+SwitchRodsType::TWSecond].loadFromProto(
            meshGenProto.switch_rods_three_way_second());
    }
    if (meshGenProto.has_guard_rails())
    {
        m_guardRailsGenData.loadFromProto(meshGenProto.guard_rails());
    }
    if (meshGenProto.has_rail_frog())
    {
        m_railFrogGenData.loadFromProto(meshGenProto.rail_frog());
    }

    bool hardCodeHeelBlockData = m_heelBlockData.directory.empty();
    if (meshGenProto.has_heel_block())
    {
        m_heelBlockData.loadFromProto(meshGenProto.heel_block());
        hardCodeHeelBlockData = false;
    }

    if (meshGenProto.has_adjunct())
    {
        m_adjunctData.loadFromProto(meshGenProto.adjunct(), m_namedSurfaces);
    }

    if (meshGenProto.has_branch_guard_rail())
    {
        m_branchGuardRailData.loadFromProto(meshGenProto.branch_guard_rail());
    }

    if (hardCodeHeelBlockData)
    {
        bool isConcrete = m_junctionSleepersGenData.nodeNamePattern.find("_c_") != std::string::npos;
        bool isWooden = m_junctionSleepersGenData.nodeNamePattern.find("_w_") != std::string::npos;

        if (isConcrete)
        {
            m_heelBlockData.rightInnerPart = "RU_str_OstryakNakladka_c_in_R.node";
            m_heelBlockData.leftInnerPart = "RU_str_OstryakNakladka_c_in_L.node";
            m_heelBlockData.outerPart = "RU_str_OstryakNakladka_c_out.node";
            m_heelBlockData.flipLeftInnerPart = false;
            m_heelBlockData.flipRightInnerPart = true;
        }
        else if (isWooden)
        {
            m_heelBlockData.rightInnerPart = "RU_str_OstryakNakladka_w_in.node";
            m_heelBlockData.leftInnerPart = "RU_str_OstryakNakladka_w_in.node";
            m_heelBlockData.outerPart = "RU_str_OstryakNakladka_w_out.node";
            m_heelBlockData.flipLeftInnerPart = false;
            m_heelBlockData.flipRightInnerPart = true;
        }
    }

    if (meshGenProto.has_embankment_profile())
    {
        getSurfaceData(+GeneratedSurfaceIdx::EmbankmentLOD0)
            .loadFromProto(meshGenProto.embankment_profile(), m_pointProfiles);
    }
    if (meshGenProto.has_embankment_profile_junctions())
    {
        getSurfaceData(+GeneratedSurfaceIdx::EmbankmentJunctions)
            .loadFromProto(meshGenProto.embankment_profile_junctions(), m_pointProfiles);
    }
    if (meshGenProto.has_rail_profile_lod0())
    {
        getSurfaceData(+GeneratedSurfaceIdx::RailsLOD0)
            .loadFromProto(meshGenProto.rail_profile_lod0(), m_pointProfiles);
    }
    if (meshGenProto.has_rail_profile_lod1())
    {
        getSurfaceData(+GeneratedSurfaceIdx::RailsLOD1)
            .loadFromProto(meshGenProto.rail_profile_lod1(), m_pointProfiles);
    }

    if (meshGenProto.has_contact_rail_profile_lod0())
    {
        getSurfaceData(+GeneratedSurfaceIdx::ContactRailLOD0)
            .loadFromProto(meshGenProto.contact_rail_profile_lod0(), m_pointProfiles);
    }

    if (meshGenProto.has_contact_rail_profile_lod1())
    {
        getSurfaceData(+GeneratedSurfaceIdx::ContactRailLOD1)
            .loadFromProto(meshGenProto.contact_rail_profile_lod1(), m_pointProfiles);
    }

    if (meshGenProto.has_guard_rail_profile_lod0())
    {
        getSurfaceData(+GeneratedSurfaceIdx::GuardRailLOD0)
            .loadFromProto(meshGenProto.guard_rail_profile_lod0(), m_pointProfiles);
    }

    if (meshGenProto.has_guard_rail_profile_lod1())
    {
        getSurfaceData(+GeneratedSurfaceIdx::GuardRailLOD1)
            .loadFromProto(meshGenProto.guard_rail_profile_lod1(), m_pointProfiles);
    }

    for (const auto& additionalSurface : meshGenProto.additional_surfaces())
    {
        if (!additionalSurface.has_name() || additionalSurface.name().empty())
        {
            continue;
        }
        m_surfaceGenData.emplace_back(std::make_unique<SurfaceGenerationData>())
            ->loadFromProto(additionalSurface, m_pointProfiles);
    }

    for (const auto& allowedOpts : meshGenProto.allowed_options())
    {
        auto& opts = m_allowedOptions[allowedOpts.name()];
        for (const auto& allowedOpt : allowedOpts.options())
        {
            opts.emplace_back(allowedOpt);
        }
    }

    return true;
}

bool RailwayMeshGeneratorConfig::fillPropertiesFromJsonFile(const std::string& meshGenConfigFile)
{
    proto::meshgen::Config meshGenProto;

    std::ifstream stream(meshGenConfigFile);
    if (!stream)
    {
        return false;
    }

    try
    {
        jsonStreamToProtoMessage(stream, meshGenProto);
    }
    catch (std::exception const& err)
    {
        Unigine::Log::error("Failed to load \"%s\": %s\n", meshGenConfigFile.data(), err.what());
        stream.close();
        return false;
    }

    return loadFromProto(meshGenProto);
}

bool
RailwayMeshGeneratorConfig::fillPropertiesFromJsonFiles(const std::string& files)
{
    std::list<std::pair<std::string, proto::meshgen::Config>> meshGenProtos;

    auto loadProto = [&meshGenProtos](const std::string& path)
    {
        std::ifstream stream(Unigine::Engine::get()->getDataPath() + path);
        if (!stream)
        {
            return false;
        }

        try
        {
            auto& meshGenProto = meshGenProtos.emplace_back(path, proto::meshgen::Config{}).second;
            jsonStreamToProtoMessage(stream, meshGenProto);
        }
        catch (std::exception const& err)
        {
            Unigine::Log::error("Failed to load \"%s\": %s\n", path.data(), err.what());
            stream.close();
            return false;
        }
        return true;
    };

    size_t offset = 0;
    std::string tokens(files);
    while ((offset = tokens.find(';')) != std::string::npos)
    {
        auto token = tokens.substr(0, offset);

        if (!loadProto(token))
        {
            return false;
        }
       
        tokens = tokens.substr(offset + 1);
    }

    if (!loadProto(tokens))
    {
        return false;
    }

    auto hasNotLoadedParent =
        [&meshGenProtos](const std::string& protoFile,
            const std::list<std::pair<std::string, proto::meshgen::Config>>::iterator& excludeIt)
    {
        for (auto protoIt = meshGenProtos.begin(); protoIt != meshGenProtos.end(); ++protoIt)
        {
            if (protoIt == excludeIt)
            {
                continue;
            }
            for (const auto& allowedOpts : protoIt->second.allowed_options())
            {
                for (const auto& allowedOpt : allowedOpts.options())
                {
                    if (allowedOpt == protoFile)
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    };

    while (!meshGenProtos.empty())
    {
        bool wasLoaded = false;
        for (auto protoIt = meshGenProtos.begin(); protoIt != meshGenProtos.end(); ++protoIt)
        {
            auto optName =
                std::filesystem::path(protoIt->first).parent_path().filename().generic_u8string();
            if (!hasNotLoadedParent(optName, protoIt))
            {
                if (!loadFromProto(protoIt->second))
                {
                    return false;
                }

                wasLoaded = true;
                meshGenProtos.erase(protoIt);
                break;
            }
        }
        if (!wasLoaded)
        {
            Unigine::Log::error("Looped dependencies in presets\"%s\": %s\n", files.data());
            return false;
        }
    }

    return true;
}

bool RailwayMeshGeneratorConfig::savePropertiesToJsonFile(const std::string& meshGenConfigFile)
{
    proto::meshgen::Config meshGenProto;

    std::ofstream stream(meshGenConfigFile);
    if (!stream)
    {
        return false;
    }

    m_commonGenData.saveToProto(*meshGenProto.mutable_common_params());
    m_branchCommonGenData.saveToProto(*meshGenProto.mutable_branch_common());
    m_junctionSleepersGenData.saveToProto(*meshGenProto.mutable_junction_sleepers());
    m_branchSleepersGenData.saveToProto(*meshGenProto.mutable_branch_sleepers());
    m_pointRailsGenData.saveToProto(*meshGenProto.mutable_point_rails());
    m_switchRodsGenData[+SwitchRodsType::Default].saveToProto(*meshGenProto.mutable_switch_rods());
    m_switchRodsGenData[+SwitchRodsType::XFront].saveToProto(
        *meshGenProto.mutable_switch_rods_x_front());
    m_switchRodsGenData[+SwitchRodsType::XBack].saveToProto(
        *meshGenProto.mutable_switch_rods_x_back());
    m_switchRodsGenData[+SwitchRodsType::XFrontLeft].saveToProto(
        *meshGenProto.mutable_switch_rods_x_front_left());
    m_switchRodsGenData[+SwitchRodsType::XFrontRight].saveToProto(
        *meshGenProto.mutable_switch_rods_x_front_right());
    m_switchRodsGenData[+SwitchRodsType::XBackLeft].saveToProto(
        *meshGenProto.mutable_switch_rods_x_back_left());
    m_switchRodsGenData[+SwitchRodsType::XBackRight].saveToProto(
        *meshGenProto.mutable_switch_rods_x_back_right());
    m_guardRailsGenData.saveToProto(*meshGenProto.mutable_guard_rails());
    m_railFrogGenData.saveToProto(*meshGenProto.mutable_rail_frog());
    m_heelBlockData.saveToProto(*meshGenProto.mutable_heel_block());
    m_adjunctData.saveToProto(*meshGenProto.mutable_adjunct());
    m_branchGuardRailData.saveToProto(*meshGenProto.mutable_branch_guard_rail());

    const auto& embankmentData = getSurfaceData(+GeneratedSurfaceIdx::EmbankmentLOD0);
    embankmentData.saveToProto(*meshGenProto.mutable_embankment_profile());

    const auto& embankmentJunctionsData = getSurfaceData(+GeneratedSurfaceIdx::EmbankmentJunctions);
    embankmentJunctionsData.saveToProto(*meshGenProto.mutable_embankment_profile_junctions());

    const auto& railDataLod0 = getSurfaceData(+GeneratedSurfaceIdx::RailsLOD0);
    railDataLod0.saveToProto(*meshGenProto.mutable_rail_profile_lod0());

    const auto& railDataLod1 = getSurfaceData(+GeneratedSurfaceIdx::RailsLOD1);
    railDataLod1.saveToProto(*meshGenProto.mutable_rail_profile_lod1());

    const auto& contactRailLOD0 = getSurfaceData(+GeneratedSurfaceIdx::ContactRailLOD0);
    contactRailLOD0.saveToProto(*meshGenProto.mutable_contact_rail_profile_lod0());

    const auto& contactRailLOD1 = getSurfaceData(+GeneratedSurfaceIdx::ContactRailLOD1);
    contactRailLOD1.saveToProto(*meshGenProto.mutable_contact_rail_profile_lod1());

    const auto& guardRailLOD0 = getSurfaceData(+GeneratedSurfaceIdx::GuardRailLOD0);
    guardRailLOD0.saveToProto(*meshGenProto.mutable_guard_rail_profile_lod0());

    const auto& guardRailLOD1 = getSurfaceData(+GeneratedSurfaceIdx::GuardRailLOD1);
    guardRailLOD1.saveToProto(*meshGenProto.mutable_guard_rail_profile_lod1());

    for (int surfaceIdx = +GeneratedSurfaceIdx::Total; surfaceIdx < m_surfaceGenData.size(); 
        ++surfaceIdx)
    {
        getSurfaceData(surfaceIdx).saveToProto(*meshGenProto.add_additional_surfaces());
    }

    for (const auto& pointProfile : m_pointProfiles)
    {
        auto& pointProfileProto = *meshGenProto.add_point_profiles();
        pointProfile->saveToProto(pointProfileProto);
    }

    for (const auto& namedSurface : m_namedSurfaces)
    {
        auto& namedSurfaceProto = *meshGenProto.add_named_surfaces();
        namedSurface->saveToProto(namedSurfaceProto);
    }

    for (const auto& [contextName, allowedOptions] : m_allowedOptions)
    {
        auto allowedOptionsProto = meshGenProto.add_allowed_options();
        allowedOptionsProto->set_name(contextName);
        for (const auto& allowedOpt : allowedOptions)
        {
            allowedOptionsProto->add_options(allowedOpt);
        }
    }

    try
    {
        protoMessageToJsonStream(meshGenProto, stream);
    }
    catch (std::exception const&)
    {
        stream.close();
        return false;
    }

    return true;
}

bool
RailwayMeshGeneratorConfig::isOptionAllowed(const std::string& optName,
    const std::vector<std::string>& contextsNames) const
{
    for (const auto& contextName : contextsNames)
    {
        auto allowedOptsIt = m_allowedOptions.find(contextName);
        if (allowedOptsIt == m_allowedOptions.end())
        {
            continue;
        }
        const auto& allowedOpts = allowedOptsIt->second;

        auto found = std::find_if(allowedOpts.begin(), allowedOpts.end(),
            [optName](const std::string& allowedOpt)
        { return stricmp(optName.data(), allowedOpt.data()) == 0; });

        if (found != allowedOpts.end())
        {
            return true;
        }
    }
    return false;
}

RailwayMeshGeneratorConfig::SurfaceGenerationData&
RailwayMeshGeneratorConfig::getSurfaceData(int index)
{
    assert(m_surfaceGenData[index] != nullptr);
    return *m_surfaceGenData[index];
}

// PointProfile

void
RailwayMeshGeneratorConfig::PointProfileData::loadFromProto(
    const proto::meshgen::PointProfile& source)
{
    loadPoints(source.points());

    LOAD_IF_HAS(source, name, name);
}

void
RailwayMeshGeneratorConfig::PointProfileData::saveToProto(
    proto::meshgen::PointProfile& source) const
{
    source.set_name(name);

    for (int pointIdx = 0; pointIdx < points.size(); ++pointIdx)
    {
        auto protoPoint = source.add_points();
        *protoPoint->mutable_coord() = unigineToProto(points[pointIdx]);
        *protoPoint->mutable_normal() = unigineToProto(normals[pointIdx]);
        *protoPoint->mutable_uv() = unigineToProto(uvCoords[pointIdx]);
    }
}

bool
RailwayMeshGeneratorConfig::PointProfileData::arePointsDifferent(
    const PointProfileData& other) const
{
    if (points.size() != other.points.size())
    {
        return true;
    }
    for (size_t pointIdx = 0; pointIdx < points.size(); ++pointIdx)
    {
        if (points[pointIdx] != other.points[pointIdx])
        {
            return true;
        }
    }
    return false;
}

void
RailwayMeshGeneratorConfig::PointProfileData::validateNormals()
{
    auto calcNormal = [](const Math::Vec3& dbinormal)
    {
        Math::vec3 binormal(dbinormal);
        binormal.normalize();
        auto normal = Math::cross(binormal, Math::vec3_right);
        return normal.normalize();
    };

    int numNormals = points.size();
    if (normals.size() != numNormals)
    {
        normals.resize(numNormals);
    }
    for (int normalIndex = 0; normalIndex < numNormals; normalIndex++)
    {
        auto& normal = normals[normalIndex];
        if (normal.length() > 0.1)
        {
            continue;
        }

        const auto& localPos = points[normalIndex];
        if (normalIndex)
        {
            const auto& prevLocalPos = points[normalIndex - 1];
            normal = calcNormal(localPos - prevLocalPos);
        }
        if (normalIndex < points.size() - 1)
        {
            const auto& nextLocalPos = points[normalIndex + 1];
            auto nextNormal = calcNormal(nextLocalPos - localPos);

            normal = normalIndex ? (normal + nextNormal).normalize() : nextNormal;
        }
    }
}

void
RailwayMeshGeneratorConfig::PointProfileData::loadPoints(
    const google::protobuf::RepeatedPtrField<proto::meshgen::ProfilePoint>&
        source)
{
    const int numChildren = source.size();
    if (!numChildren)
    {
        return;
    }

    points.clear();
    normals.clear();
    uvCoords.clear();

    points.reserve(numChildren);
    uvCoords.reserve(numChildren);
    normals.reserve(numChildren);

    const bool fippedTextureV = Unigine::Render::isFlipped();

    if (fippedTextureV)
    {
        for (const auto& point : source)
        {
            points.push_back(protoToUnigine(point.coord()));

            Math::vec2 textureCoord(protoToUnigine(point.uv()));

            textureCoord.y = 1.0f - textureCoord.y;

            uvCoords.push_back(textureCoord);

            normals.push_back(protoToUnigine(point.normal()));
        }
    }
    else
    {
        for (const auto& point : source)
        {
            points.push_back(protoToUnigine(point.coord()));
            uvCoords.push_back(protoToUnigine(point.uv()));
            normals.push_back(protoToUnigine(point.normal()));
        }
    }

    validateNormals();
}

// Surface

void assign(Unigine::Math::vec3& dst, const proto::Vec3f& src)
{
    dst.set(src.x(), src.y(), src.z());
}

void RailwayMeshGeneratorConfig::SurfaceGenerationData::loadFromProto(
    const proto::meshgen::Surface& source, std::vector<PointProfileDataPtr>& pointProfiles)
{
    if (source.has_enable() && !source.enable())
    {
        pointProfile = nullptr;
        return;
    }

    if (source.has_point_profile_name())
    {
        const auto& profileName = source.point_profile_name();
        auto profileIt = std::find_if(pointProfiles.begin(), pointProfiles.end(),
            [&profileName](const PointProfileDataPtr& pointProfile)
            {
                return (stricmp(profileName.data(), pointProfile->name.data()) == 0);
            });

        if (profileIt == pointProfiles.end())
        {
            return;
        }
        pointProfile = *profileIt;
    }
    else
    {
        if (!pointProfile)
        {
            pointProfiles.emplace_back(new PointProfileData);
            pointProfile = pointProfiles.back();

            static int legacyPointProfileIdx = 0;
            pointProfile->name = "tempPointProfile" + std::to_string(legacyPointProfileIdx++);
        }
        pointProfile->loadPoints(source.points());
    }

    loadCustomMaterials(source);

    LOAD_IF_HAS(source, name, name);
    LOAD_IF_HAS(source, num_segments_per_unit, numSegmentsPerUnit);
    LOAD_IF_HAS(source, tex_coords_v_step, texCoordVStep);
    LOAD_IF_HAS(source, min_visible_distance, minVisibleDistance);
    LOAD_IF_HAS(source, max_visible_distance, maxVisibleDistance);
    LOAD_IF_HAS(source, min_fade_distance, minFadeDistance);
    LOAD_IF_HAS(source, max_fade_distance, maxFadeDistance);
    LOAD_IF_HAS(source, cap_node_path, capNodePath);
    ASSIGN_IF_HAS(source, profile_offset, profileOffset);

    if (source.has_default_material_info())
    {
        defaultMaterialInfo.loadFromProto(source.default_material_info());
    }

    for (const auto& singleCollisionMask : source.collision_masks())
    {
        collisionMask |= 1 << singleCollisionMask;
    }
}

void RailwayMeshGeneratorConfig::SurfaceGenerationData::
saveToProto(proto::meshgen::Surface& source) const
{
    source.set_name(name);
    if (pointProfile)
    {
        source.set_point_profile_name(pointProfile->name);
    }
    source.set_num_segments_per_unit(numSegmentsPerUnit);
    source.set_tex_coords_v_step(texCoordVStep);
    source.set_min_visible_distance(minVisibleDistance);
    source.set_max_visible_distance(maxVisibleDistance);
    source.set_min_fade_distance(minFadeDistance);
    source.set_max_fade_distance(maxFadeDistance);

    for (const auto& customMaterial : customMaterials)
    {
        auto protoCustomMaterial = source.add_custom_materials();
        *protoCustomMaterial->mutable_surface_name() = customMaterial.first;
        *protoCustomMaterial->mutable_material() = customMaterial.second;
    }

    defaultMaterialInfo.saveToProto(*source.mutable_default_material_info());

    source.set_cap_node_path(capNodePath);

    auto profile_offset = source.mutable_profile_offset();
    profile_offset->set_x(profileOffset.x);
    profile_offset->set_y(profileOffset.y);
    profile_offset->set_z(profileOffset.z);

    for (int bit = 0; bit < 32; ++bit)
    {
        if (collisionMask & (1 << bit))
        {
            source.add_collision_masks(bit);
        }
    }
}

void RailwayMeshGeneratorConfig::SurfaceGenerationData::
loadCustomMaterials(const proto::meshgen::Surface& source)
{
    for (const auto& material : source.custom_materials())
    {
        customMaterials.emplace(material.surface_name(), material.material());
    }
}

const std::vector<std::array<int, 3>>& 
RailwayMeshGeneratorConfig::SurfaceGenerationData::getCapTriangles()
{
    std::unique_lock lock(m_capTrianglesProtect);
    if (m_capTriangles.empty())
    {
        std::vector<CDT::V2d<double>> vertsCDT;
        std::vector<CDT::Edge> edgesCDT;

        const int numProfilePoints = pointProfile->points.size();
        for (int profilePointIdx = 0; profilePointIdx < numProfilePoints; ++profilePointIdx)
        {
            auto vert = pointProfile->points[profilePointIdx];

            vertsCDT.push_back(CDT::V2d<double>::make(vert.y, vert.z));

            if (profilePointIdx == numProfilePoints - 1)
            {
                edgesCDT.push_back(CDT::Edge(profilePointIdx, 0));
            }
            else
            {
                edgesCDT.push_back(CDT::Edge(profilePointIdx, profilePointIdx + 1));
            }
        }

        auto indexMap = CDT::RemoveDuplicatesAndRemapEdges(vertsCDT, edgesCDT);
        auto remapIndex = [&indexMap](size_t newIndex)
        {
            for (int oldIndex = 0; oldIndex < indexMap.size(); ++oldIndex)
            {
                if (indexMap[oldIndex] == newIndex)
                    return oldIndex;
            }
            return 0;
        };

        Triangulation cdt(CDT::FindingClosestPoint::BoostRTree);
        cdt.insertVertices(vertsCDT);
        cdt.insertEdges(edgesCDT);
        cdt.eraseOuterTriangles();

        for (auto& tri : cdt.triangles)
        {
            std::array<int, 3> addTri = {
                remapIndex(tri.vertices[0]),
                remapIndex(tri.vertices[1]),
                remapIndex(tri.vertices[2])
            };
            m_capTriangles.push_back(std::move(addTri));
        }
    }
    return m_capTriangles;
}

//-------------------------------------------------------------------------------------
// Generation data components.
//-------------------------------------------------------------------------------------
void RailwayMeshGeneratorConfig::JunctionSleepersGenerationData::
loadFromProto(const proto::meshgen::JunctionSleepers& source)
{
    LOAD_IF_HAS(source, directory, directory);
    LOAD_IF_HAS(source, mount_node_path, mountNodePath);
    LOAD_IF_HAS(source, node_name_pattern, nodeNamePattern);
    LOAD_IF_HAS(source, min_length, minLength);
    LOAD_IF_HAS(source, max_length, maxLength);
    LOAD_IF_HAS(source, inc_step, lengthIncreaseStep);
    LOAD_IF_HAS(source, start_offset, startOffset);
    LOAD_IF_HAS(source, interval, interval);
    LOAD_IF_HAS(source, visible_distance, visibleDistance);
    LOAD_IF_HAS(source, point_rails_first_interval, pointRailsFirstInterval);
    LOAD_IF_HAS(source, sleeper_width, sleeperWidth);
    LOAD_IF_HAS(source, single_sided_mount_node_path, singleSidedMountNodePath);
    LOAD_IF_HAS(source, switch_box_interval, switchBoxInterval);
    LOAD_IF_HAS(source, random_angle, randomAngle);
    LOAD_IF_HAS(source, add_random_angle, addRandomAngle);
    LOAD_IF_HAS(source, first_junction_sleeper, firstJunctionSleeper);
    LOAD_IF_HAS(source, second_junction_sleeper, secondJunctionSleeper);

    if (source.has_default_material_info())
    {
        defaultMaterialInfo.loadFromProto(source.default_material_info());
    }
}

void
RailwayMeshGeneratorConfig::JunctionSleepersGenerationData::
saveToProto(proto::meshgen::JunctionSleepers& source) const
{
    source.set_directory(directory);
    source.set_mount_node_path(mountNodePath);
    source.set_node_name_pattern(nodeNamePattern);
    source.set_min_length(minLength);
    source.set_max_length(maxLength);
    source.set_inc_step(lengthIncreaseStep);
    source.set_start_offset(startOffset);
    source.set_interval(interval);
    source.set_visible_distance(visibleDistance);
    source.set_point_rails_first_interval(pointRailsFirstInterval);
    source.set_sleeper_width(sleeperWidth);
    source.set_single_sided_mount_node_path(singleSidedMountNodePath);
    source.set_random_angle(randomAngle);
    source.set_add_random_angle(addRandomAngle);
    source.set_first_junction_sleeper(firstJunctionSleeper);
    source.set_second_junction_sleeper(secondJunctionSleeper);

    defaultMaterialInfo.saveToProto(*source.mutable_default_material_info());
}

void RailwayMeshGeneratorConfig::BranchSleepersGenerationData::
loadFromProto(const proto::meshgen::BranchSleepers& source)
{
    if (source.has_enable() && !source.enable())
    {
        return;
    }

    LOAD_IF_HAS(source, directory, directory);
    LOAD_IF_HAS(source, node_name_pattern, nodeNamePattern);
    LOAD_IF_HAS(source, variants_count, variantsCount);
    LOAD_IF_HAS(source, add_random_angle, addRandomAngle);
    LOAD_IF_HAS(source, random_angle, randomAngle);

    if (source.has_default_material_info())
    {
        defaultMaterialInfo.loadFromProto(source.default_material_info());
    }
}

void RailwayMeshGeneratorConfig::BranchSleepersGenerationData::
saveToProto(proto::meshgen::BranchSleepers& source) const
{
    source.set_enable(!directory.empty());
    source.set_directory(directory);
    source.set_node_name_pattern(nodeNamePattern);
    source.set_variants_count(variantsCount);
    source.set_random_angle(randomAngle);
    source.set_add_random_angle(addRandomAngle);

    defaultMaterialInfo.saveToProto(*source.mutable_default_material_info());
}

std::string RailwayMeshGeneratorConfig::BranchSleepersGenerationData::getTemplatePath(int variant) const
{
    std::string namePatten = nodeNamePattern;
    int asteriskPos = namePatten.find_first_of('*', 0);
    std::stringstream ss;
    ss << directory;
    if ((directory.back() != '\\') && (directory.back() != '/'))
    {
        ss << "\\";
    }
    ss << namePatten.substr(0, asteriskPos);
    ss << variant;
    ss << namePatten.substr(asteriskPos + 1);

    return ss.str();
}

bool
RailwayMeshGeneratorConfig::PointRailGenerationData::loadAvailableLengths()
{
    size_t firstAsteriskOffset = nodeNamePattern.find_first_of("*");
    if (firstAsteriskOffset == std::string::npos)
    {
        return false;
    }
    size_t lastAsteriskOffset = nodeNamePattern.find_last_of("*");
    if (lastAsteriskOffset == std::string::npos)
    {
        return false;
    }
    if (firstAsteriskOffset == lastAsteriskOffset)
    {
        return false;
    }

    std::string filesPath(Engine::get()->getDataPath());
    filesPath += "/" + directory;
    std::replace(filesPath.begin(), filesPath.end(), '\\', '/');

    auto filesDir = Dir::create();
    if (!filesDir->open(filesPath.data()))
    {
        return false;
    }

    std::string patternStr(nodeNamePattern, 0, firstAsteriskOffset);
    patternStr += "([LR])";
    patternStr += std::string(nodeNamePattern, firstAsteriskOffset + 1,
        lastAsteriskOffset - firstAsteriskOffset - 1);
    patternStr += "([\\d]*)";
    patternStr += std::string(nodeNamePattern, lastAsteriskOffset + 1);
    std::regex pattern(patternStr);

    String requiredExtension = String::extension(nodeNamePattern.data());
    for (int fileIndex = 0, fileCount = filesDir->getNumFiles(); fileIndex < fileCount;
            ++fileIndex)
    {
        std::string fileName = filesDir->getFileName(fileIndex);
        if (String::extension(fileName.data()) != requiredExtension)
        {
            continue;
        }
        std::replace(fileName.begin(), fileName.end(), '\\', '/');
        std::string relFileName = fileName.substr(filesPath.length() + 1);

        std::smatch matches;
        if (std::regex_search(relFileName, matches, pattern))
        {
            int lengthMM = std::stoi(matches[2]);
            if (matches[1] == "L")
            {
                if (std::find(m_leftAvailableLengthsMM.begin(), m_leftAvailableLengthsMM.end(),
                        lengthMM) == m_leftAvailableLengthsMM.end())
                {
                    m_leftAvailableLengthsMM.push_back(lengthMM);
                }
            }
            else
            {
                if (std::find(m_rightAvailableLengthsMM.begin(), m_rightAvailableLengthsMM.end(),
                        lengthMM) == m_rightAvailableLengthsMM.end())
                {
                    m_rightAvailableLengthsMM.push_back(lengthMM);
                }
            }
        }
    }

    filesDir->close();

    std::sort(m_leftAvailableLengthsMM.begin(), m_leftAvailableLengthsMM.end(),
        std::greater<int>());
    std::sort(m_rightAvailableLengthsMM.begin(), m_rightAvailableLengthsMM.end(),
        std::greater<int>());

    return true;
}

int
RailwayMeshGeneratorConfig::PointRailGenerationData::chooseSuitablePointRailLength(
    int availableSpaceMM, bool leftSide) const
{
    if (m_leftAvailableLengthsMM.empty() || m_rightAvailableLengthsMM.empty())
    {
        return 0;
    }
    return chooseSuitableLength(availableSpaceMM,
        leftSide ? m_leftAvailableLengthsMM : m_rightAvailableLengthsMM);
}

std::string
RailwayMeshGeneratorConfig::PointRailGenerationData::getPointRailPath(int lengthMM,
    bool leftSide) const
{
    if (nodeNamePattern.empty())
    {
        return "";
    }

    int firstAsteriscPos = nodeNamePattern.find_first_of('*');
    int secondAsteriscPos = nodeNamePattern.find_last_of('*');

    assert(firstAsteriscPos != std::string::npos);
    assert(secondAsteriscPos != std::string::npos);
    assert(secondAsteriscPos != firstAsteriscPos);

    std::string part1 = nodeNamePattern.substr(0, firstAsteriscPos);
    std::string part2 =
        nodeNamePattern.substr(firstAsteriscPos + 1, (secondAsteriscPos - firstAsteriscPos - 1));
    std::string part3 = nodeNamePattern.substr(secondAsteriscPos + 1);

    std::stringstream ss;
    ss << directory << "/";
    ss << part1;
    ss << (leftSide ? "L" : "R");
    ss << part2 << lengthMM << part3;

    return ss.str();
}

void RailwayMeshGeneratorConfig::PointRailGenerationData::
loadFromProto(const proto::meshgen::PointRail& source)
{
    LOAD_IF_HAS(source, directory, directory);
    LOAD_IF_HAS(source, node_name_pattern, nodeNamePattern);
    LOAD_IF_HAS(source, stock_offset, stockOffset);
    LOAD_IF_HAS(source, x_middle_gap, xMiddleGap);
    LOAD_IF_HAS(source, x_narrow_sharp_frog_max_offset, xNarrowSharpFrogMaxOffset);

    loadAvailableLengths();

    if (source.has_material_info())
    {
        materialInfo.loadFromProto(source.material_info());
    }
}

void RailwayMeshGeneratorConfig::PointRailGenerationData::
saveToProto(proto::meshgen::PointRail& source) const
{
    source.set_directory(directory);
    source.set_node_name_pattern(nodeNamePattern);
    source.set_stock_offset(stockOffset);
    source.set_x_middle_gap(xMiddleGap);
    source.set_x_narrow_sharp_frog_max_offset(xNarrowSharpFrogMaxOffset);

    materialInfo.saveToProto(*source.mutable_material_info());
}

void RailwayMeshGeneratorConfig::SwitchRodGenerationData::
loadFromProto(const proto::meshgen::SwitchRods& source)
{
    LOAD_IF_HAS(source, rod_node_path, rodNodePath);
    LOAD_IF_HAS(source, head_rod_node_path_left, headRodNodePathLeft);
    LOAD_IF_HAS(source, head_rod_node_path_right, headRodNodePathRight);
    LOAD_IF_HAS(source, switch_box_node_path_left, switchBoxNodePathLeft);
    LOAD_IF_HAS(source, switch_box_node_path_right, switchBoxNodePathRight);
    LOAD_IF_HAS(source, switch_box_prefer_left, switchBoxPreferLeft);
    LOAD_IF_HAS(source, border_bar_node_path, borderBarPath);
}

void RailwayMeshGeneratorConfig::SwitchRodGenerationData::
saveToProto(proto::meshgen::SwitchRods& source) const
{
    source.set_rod_node_path(rodNodePath);
    source.set_head_rod_node_path_left(headRodNodePathLeft);
    source.set_head_rod_node_path_right(headRodNodePathRight);
    source.set_switch_box_node_path_left(switchBoxNodePathLeft);
    source.set_switch_box_node_path_right(switchBoxNodePathRight);
    source.set_switch_box_prefer_left(switchBoxPreferLeft);
    source.set_border_bar_node_path(borderBarPath);
}

const std::string&
RailwayMeshGeneratorConfig::SwitchRodGenerationData::getHeadRodNodePath(
    std::optional<bool> forceLeft) const
{
    if (forceLeft == std::nullopt)
    {
        if (switchBoxPreferLeft)
        {
            return headRodNodePathLeft;
        }
        return headRodNodePathRight;
    }
    if (*forceLeft)
    {
        return headRodNodePathLeft;
    }
    return headRodNodePathRight;
}

RailwayMeshGeneratorConfig::SwitchRodGenerationData::SwitchBoxNodeInfo
RailwayMeshGeneratorConfig::SwitchRodGenerationData::getSwitchBoxNodeInfo(
    std::optional<bool> forceLeft) const
{
    if (forceLeft == std::nullopt)
    {
        if (switchBoxPreferLeft)
        {
            return RailwayMeshGeneratorConfig::SwitchRodGenerationData::SwitchBoxNodeInfo{true,
                switchBoxNodePathLeft};
        }
        return RailwayMeshGeneratorConfig::SwitchRodGenerationData::SwitchBoxNodeInfo{false,
            switchBoxNodePathRight};
    }
    if (*forceLeft)
    {
        return RailwayMeshGeneratorConfig::SwitchRodGenerationData::SwitchBoxNodeInfo{true,
            switchBoxNodePathLeft};
    }
    return RailwayMeshGeneratorConfig::SwitchRodGenerationData::SwitchBoxNodeInfo{false,
        switchBoxNodePathRight};
}

void RailwayMeshGeneratorConfig::GuardRailsGenerationData::
loadFromProto(const proto::meshgen::GuardRails& source)
{
    LOAD_IF_HAS(source, directory, directory);
    LOAD_IF_HAS(source, left_node, leftNode);
    LOAD_IF_HAS(source, right_node, rightNode);
    LOAD_IF_HAS(source, middle_node, middleNode);
    LOAD_IF_HAS(source, mount_node, mountNode);
    LOAD_IF_HAS(source, offset, offset);
    LOAD_IF_HAS(source, start_gap, startGap);
    LOAD_IF_HAS(source, end_gap, endGap);
    LOAD_IF_HAS(source, visible_distance, visibleDistance);
}

void RailwayMeshGeneratorConfig::GuardRailsGenerationData::
saveToProto(proto::meshgen::GuardRails& source) const
{
    source.set_directory(directory);
    source.set_left_node(leftNode);
    source.set_right_node(rightNode);
    source.set_middle_node(middleNode);
    source.set_mount_node(mountNode);
    source.set_offset(offset);
    source.set_start_gap(startGap);
    source.set_end_gap(endGap);
    source.set_visible_distance(visibleDistance);
}

void RailwayMeshGeneratorConfig::RailFrogGenerationData::
loadFromProto(const proto::meshgen::RailFrog& source)
{
    LOAD_IF_HAS(source, directory, directory);
    LOAD_IF_HAS(source, start_gap, startGap);
    LOAD_IF_HAS(source, end_gap, endGap);
    LOAD_IF_HAS(source, mounts_offset, mountsOffset);
    LOAD_IF_HAS(source, mounts_offset_obtuse, mountsOffsetObtuse);
    LOAD_IF_HAS(source, skinned, skinned);

    if (source.has_material_info())
    {
        materialInfo.loadFromProto(source.material_info());
    }
}

void RailwayMeshGeneratorConfig::RailFrogGenerationData::
saveToProto(proto::meshgen::RailFrog& source) const
{
    source.set_mounts_offset_obtuse(mountsOffsetObtuse);
    source.set_mounts_offset(mountsOffset);
    source.set_directory(directory);
    source.set_start_gap(startGap);
    source.set_end_gap(endGap);
    source.set_skinned(skinned);

    materialInfo.saveToProto(*source.mutable_material_info());
}

void RailwayMeshGeneratorConfig::HeelBlockGenerationData::
loadFromProto(const proto::meshgen::HeelBlock& source)
{
    LOAD_IF_HAS(source, directory, directory);
    LOAD_IF_HAS(source, left_inner_part, leftInnerPart);
    LOAD_IF_HAS(source, left_inner_flip, flipLeftInnerPart);
    LOAD_IF_HAS(source, right_inner_part, rightInnerPart);
    LOAD_IF_HAS(source, right_inner_flip, flipRightInnerPart);
    LOAD_IF_HAS(source, middle_part, middlePart);
    LOAD_IF_HAS(source, outer_part, outerPart);
    LOAD_IF_HAS(source, half_size, halfSize);

    if (source.has_material_info())
    {
        materialInfo.loadFromProto(source.material_info());
    }
}

void RailwayMeshGeneratorConfig::HeelBlockGenerationData::
saveToProto(proto::meshgen::HeelBlock& source) const
{
    source.set_directory(directory);
    source.set_left_inner_part(leftInnerPart);
    source.set_left_inner_flip(flipLeftInnerPart);
    source.set_right_inner_part(rightInnerPart);
    source.set_right_inner_flip(flipRightInnerPart);
    source.set_middle_part(middlePart);
    source.set_outer_part(outerPart);

    materialInfo.saveToProto(*source.mutable_material_info());
}

void RailwayMeshGeneratorConfig::CommonGenerationData::
loadFromProto(const proto::meshgen::Common& source)
{
    LOAD_IF_HAS(source, railway_track_width, railwayTrackWidth);
    LOAD_IF_HAS(source, max_path_straight_treshold, maxPathStraightTreshold);
    LOAD_IF_HAS(source, min_path_similarity_percent, minPathSimilarityPercent);

    if (source.has_sleepers_material_info())
    {
        sleepersMaterialInfo.loadFromProto(source.sleepers_material_info());
    }
    if (source.has_blend_embankment_material_info())
    {
        blendEmbankmentMaterialInfo.loadFromProto(source.blend_embankment_material_info());
    }
}

void RailwayMeshGeneratorConfig::CommonGenerationData::
saveToProto(proto::meshgen::Common& source) const
{
    source.set_railway_track_width(railwayTrackWidth);
    source.set_max_path_straight_treshold(maxPathStraightTreshold);
    source.set_min_path_similarity_percent(minPathSimilarityPercent);

    sleepersMaterialInfo.saveToProto(*source.mutable_sleepers_material_info());
    blendEmbankmentMaterialInfo.saveToProto(*source.mutable_blend_embankment_material_info());
}

void RailwayMeshGeneratorConfig::BranchCommonGenerationData::
loadFromProto(const proto::meshgen::BranchCommon& source)
{
    LOAD_IF_HAS(source, gap_step, gapStep);
    LOAD_IF_HAS(source, gap_length, gapLength);
    LOAD_IF_HAS(source, fish_plate_node, fishPlateNode);
    LOAD_IF_HAS(source, max_segment_length, maxSegmentLength);

    if (source.has_fish_plate_material_info())
    {
        fishPlateMaterialInfo.loadFromProto(source.fish_plate_material_info());
    }
}

void RailwayMeshGeneratorConfig::BranchCommonGenerationData::
saveToProto(proto::meshgen::BranchCommon& source) const
{
    source.set_gap_step(gapStep);
    source.set_gap_length(gapLength);
    source.set_fish_plate_node(fishPlateNode);
    source.set_max_segment_length(maxSegmentLength);

    fishPlateMaterialInfo.saveToProto(*source.mutable_fish_plate_material_info());
}

//

void
RailwayMeshGeneratorConfig::AdjunctPartData::loadFromProto(
    const proto::meshgen::AdjunctPart& source,
    const std::vector<SurfaceGenerationDataPtr>& namedSurfaces)
{
    if (name.empty())
    {
        name = source.name();
    }

    if (source.surface_names().size())
    {
        surfaces.clear();
        for (const auto& surfaceName : source.surface_names())
        {
            auto surfaceIt = std::find_if(namedSurfaces.begin(), namedSurfaces.end(),
                [&surfaceName](const SurfaceGenerationDataPtr& surface)
                { return (stricmp(surfaceName.data(), surface->name.data()) == 0); });

            if (surfaceIt != namedSurfaces.end())
            {
                surfaces.emplace_back(*surfaceIt);
            }
        }
    }

    LOAD_IF_HAS(source, enabled, enabled);

    LOAD_IF_HAS(source, front_distance_offset, frontDistanceOffsetMm);
    LOAD_IF_HAS(source, back_distance_offset, backDistanceOffsetMm);

    LOAD_IF_HAS(source, baseline_offset_height, baselineOffsetHeightMm);
    LOAD_IF_HAS(source, baseline_offset_side, baselineOffsetSideMm);

    LOAD_IF_HAS(source, enable_braces, enableBraces);
    LOAD_IF_HAS(source, brace_node_path, braceNodePath);
    LOAD_IF_HAS(source, brace_interval, braceIntervalMm);
    LOAD_IF_HAS(source, brace_gap, braceGapMm);
    LOAD_IF_HAS(source, first_brace_distance_offset, firstBraceDistanceOffsetMm);
    LOAD_IF_HAS(source, second_brace_distance_offset, secondBraceDistanceOffsetMm);
    LOAD_IF_HAS(source, penult_brace_distance_offset, penultBraceDistanceOffsetMm);
    LOAD_IF_HAS(source, last_brace_distance_offset, lastBraceDistanceOffsetMm);
    LOAD_IF_HAS(source, rotate_braces, rotateBraces);
    LOAD_IF_HAS(source, fit_braces_to_sleepers, fitBracesToSleepers);
    LOAD_IF_HAS(source, brace_sleeper_offset, braceSleeperOffsetMm);

    LOAD_IF_HAS(source, enable_front_cap, enableFrontCap);
    LOAD_IF_HAS(source, front_cap_node_path, frontCapNodePath);

    LOAD_IF_HAS(source, enable_back_cap, enableBackCap);
    LOAD_IF_HAS(source, back_cap_node_path, backCapNodePath);
}

void
RailwayMeshGeneratorConfig::AdjunctPartData::saveToProto(proto::meshgen::AdjunctPart& source) const
{
    for (const auto& surface : surfaces)
    {
        source.add_surface_names(surface->name);
    }

    source.set_front_distance_offset(frontDistanceOffsetMm);
    source.set_back_distance_offset(backDistanceOffsetMm);

    source.set_baseline_offset_height(baselineOffsetHeightMm);
    source.set_baseline_offset_side(baselineOffsetSideMm);

    source.set_enable_braces(enableBraces);
    if (!braceNodePath.empty())
    {
        source.set_brace_node_path(braceNodePath);
    }
    source.set_brace_interval(braceIntervalMm);
    source.set_brace_gap(braceGapMm);
    source.set_first_brace_distance_offset(firstBraceDistanceOffsetMm);
    source.set_second_brace_distance_offset(secondBraceDistanceOffsetMm);
    source.set_penult_brace_distance_offset(penultBraceDistanceOffsetMm);
    source.set_last_brace_distance_offset(lastBraceDistanceOffsetMm);
    source.set_rotate_braces(rotateBraces);
    source.set_fit_braces_to_sleepers(fitBracesToSleepers);

    source.set_enable_front_cap(enableFrontCap);
    if (!braceNodePath.empty())
    {
        source.set_front_cap_node_path(frontCapNodePath);
    }
    source.set_enable_back_cap(enableBackCap);
    if (!braceNodePath.empty())
    {
        source.set_back_cap_node_path(backCapNodePath);
    }
}

//

void
RailwayMeshGeneratorConfig::AdjunctData::loadFromProto(const proto::meshgen::Adjunct& source,
    const std::vector<SurfaceGenerationDataPtr>& namedSurfaces)
{
    for (const auto& partProto : source.parts())
    {
        auto partIt = std::find_if(parts.begin(), parts.end(),
            [&partProto](const AdjunctPartData& part)
            { return stricmp(part.name.data(), partProto.name().data()) == 0; });

        if (partIt == parts.end())
        {
            parts.emplace_back();
            partIt = std::prev(parts.end());
            if (partIt == parts.end())
            {
                assert(partIt != parts.end());
                continue;
            }
        }

        partIt->loadFromProto(partProto, namedSurfaces);
    }
}

void
RailwayMeshGeneratorConfig::AdjunctData::saveToProto(proto::meshgen::Adjunct& source) const
{
    for (const auto& part : parts)
    {
        part.saveToProto(*source.add_parts());
    }
}

void RailwayMeshGeneratorConfig::BranchGuardRailData::loadFromProto(proto::meshgen::BranchGuardRail const& source)
{
    LOAD_IF_HAS(source, end_part_length, endPartLengthMm);
    LOAD_IF_HAS(source, rail_gap, railGapMm);
    LOAD_IF_HAS(source, fish_plate_node, fishPlateNode);
    LOAD_IF_HAS(source, mount_node, mountNode);

    if (source.has_fish_plate_material_info())
    {
        fishPlateMaterialInfo.loadFromProto(source.fish_plate_material_info());
    }
}

void RailwayMeshGeneratorConfig::BranchGuardRailData::saveToProto(proto::meshgen::BranchGuardRail source) const
{
    source.set_end_part_length(endPartLengthMm);
    source.set_rail_gap(railGapMm);
    source.set_fish_plate_node(fishPlateNode);
    source.set_mount_node(mountNode);

    fishPlateMaterialInfo.saveToProto(*source.mutable_fish_plate_material_info());
}
