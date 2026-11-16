#include "DynamicRailwayPartGenerated.h"

#include <UnigineEngine.h>
using namespace Unigine;

namespace
{

std::mutex configLocksProtect;
std::unordered_map<std::string, std::mutex> configCacheProtects;


/** Weak configs will free memory in editors, which switch often configs by not storing not used
 * configs, but this slow down parts loading while moving in runtime - few used configs can be
 * stored all the time, they also should be prloaded in such case.
 */
#ifdef DYNAMIC_RAILWAYS_WEAK_CONFIGS
std::unordered_map<std::string, RailwayMeshGeneratorConfigWeak> configCache;
#else
std::unordered_map<std::string, RailwayMeshGeneratorConfigPtr> configCache;
#endif

}//Anonymous namespace

RailwayMeshGeneratorConfigPtr DynamicRailwayPartGenerated::getOrLoadRailwayMeshGeneratorConfig(
    const std::string& path)
{
    if (path.empty())
    {
        return {};
    }

    std::unordered_map<std::string, std::mutex>::iterator protectIt;
    {
        std::unique_lock lock(configLocksProtect);

        protectIt = configCacheProtects.find(path);
        if (protectIt == configCacheProtects.end())
        {
            protectIt = configCacheProtects
                    .emplace(std::piecewise_construct, std::make_tuple(path), std::make_tuple())
                    .first;
        }
    }

    std::unique_lock lock(protectIt->second);

    auto configIt = configCache.find(path);
    if (configIt != configCache.end())
    {
#ifdef DYNAMIC_RAILWAYS_WEAK_CONFIGS
        if (auto config = configIt->second.lock())
        {
            return config;
        }
        configCache.erase(configIt);
#else
        return configIt->second;
#endif
    }

    auto config = std::make_shared<RailwayMeshGeneratorConfig>();
    if (!config->fillPropertiesFromJsonFiles(path))
    {
        return {};
    }
    configCache.emplace(path, RailwayMeshGeneratorConfigWeak(config));
    return config;
}

DynamicRailwayPartGenerated::DynamicRailwayPartGenerated(uint32_t id, uint32_t subId,
    const std::string& configPath, const RailwayMeshGeneratorConfigPtr& config)
    : DynamicRailwayPart(id, subId)
    , m_configPath(configPath)
    , m_config(config)
{
}

bool DynamicRailwayPartGenerated::isDifferentWith(const DynamicRailwayPartPtr& old) const
{
    auto generated = dynamic_cast<DynamicRailwayPartGenerated*>(old.get());

    if (!generated)
    {
        return true;
    }

    if (generated->m_configPath != m_configPath)
    {
        return true;
    }

    return false;
}

bool DynamicRailwayPartGenerated::loadConfig()
{
    m_config = getOrLoadRailwayMeshGeneratorConfig(m_configPath);
    return (m_config != nullptr);
}

bool
DynamicRailwayPartGenerated::hasDifferentEmbankmentProfile(
    const RailwayMeshGeneratorConfigPtr& otherConfig)
{
    if (!m_config || !otherConfig)
    {
        return true;
    }
    const auto& thisEmbankment = m_config->getSurfaceData(
        +RailwayMeshGeneratorConfig::GeneratedSurfaceIdx::EmbankmentLOD0);
    const auto& otherEmbankment = otherConfig->getSurfaceData(
        +RailwayMeshGeneratorConfig::GeneratedSurfaceIdx::EmbankmentLOD0);

    if (!thisEmbankment.pointProfile)
    {
        if (otherEmbankment.pointProfile)
        {
            return false;
        }
        return true;
    }
    if (!otherEmbankment.pointProfile)
    {
        if (thisEmbankment.pointProfile)
        {
            return false;
        }
        return true;
    }
    return thisEmbankment.pointProfile->arePointsDifferent(*otherEmbankment.pointProfile);
}

// Debugging

#ifdef DYNAMIC_RAILWAYS_DEBUG
void DynamicRailwayPartGenerated::setDebugLogger(const ObjectMessageLoggerPtr& logger)
{
    m_debugLogger = logger;
}

void DynamicRailwayPartGenerated::setDebugVisualizer(const ObjectDebugVisualizerPtr& visualizer)
{
    m_debugVisualizer = visualizer;
}
#endif

void DynamicRailwayPartGenerated::message(const char* format, ...)
{
#ifdef DYNAMIC_RAILWAYS_DEBUG
    if (m_debugLogger)
    {
        va_list argptr;
        va_start(argptr, format);
        m_debugLogger->debug(format, argptr);
        va_end(argptr);
    }
#endif
}

void DynamicRailwayPartGenerated::warning(const char* format, ...)
{
#ifdef DYNAMIC_RAILWAYS_DEBUG
    if (m_debugLogger)
    {
        va_list argptr;
        va_start(argptr, format);
        m_debugLogger->warning(format, argptr);
        va_end(argptr);
    }
#endif
}

void DynamicRailwayPartGenerated::error(const char* format, ...)
{
#ifdef DYNAMIC_RAILWAYS_DEBUG
    if (m_debugLogger)
    {
        va_list argptr;
        va_start(argptr, format);
        m_debugLogger->error(format, argptr);
        va_end(argptr);
    }
#endif
}
