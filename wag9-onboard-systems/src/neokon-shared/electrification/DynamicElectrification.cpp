#include "DynamicElectrification.h"

#include <UnigineEngine.h>
#include <UnigineMathLib.h>

using namespace Unigine;

//static
DynamicElectrification* DynamicElectrification::s_instance{ nullptr };

//static
void DynamicElectrification::registerClass()
{
    ObjectExternBase::addClassID<DynamicRailways>(+ObjectExternIds::DynamicElectrification);
}

//static
DynamicElectrification*
DynamicElectrification::create(DynamicElectrificationGetDataFunc const& getDataFunc)
{
    auto getDataFuncRailways = [getDataFunc](const Unigine::Math::Vec3& center)
        -> DynamicRailwayParts
    {
        auto parts = getDataFunc(center);

        DynamicRailwayParts result;
        result.insert(result.begin(), parts.cbegin(), parts.cend());

        return result;
    };

    return new DynamicElectrification(getDataFuncRailways);
}

//static
DynamicElectrification* DynamicElectrification::instance()
{
    return s_instance;
}

DynamicElectrification::DynamicElectrification(void* object)
    : DynamicRailways(object)
{
}

DynamicElectrification::~DynamicElectrification()
{
    s_instance = nullptr;
}

DynamicElectrification::DynamicElectrification(const DynamicRailwaysGetDataFunc& getDataFunc)
    : DynamicRailways(getDataFunc, "DynamicElectrificationParts")
{

#ifdef DYNAMIC_ELECTRIFICATION_DEBUG
    m_debugLogger = std::make_shared<ObjectMessageLogger>();
    m_debugVisualizer = std::make_shared<ObjectDebugVisualizer>();
#endif

    if (!s_instance)
    {
        s_instance = this;
    }
}

void DynamicElectrification::update(float ifps)
{
    DynamicRailways::update(ifps);

#ifdef DYNAMIC_ELECTRIFICATION_DEBUG
    m_debugVisualizer->visualize(Math::Mat4::IDENTITY);
#endif
}

ElectrificationMeshGeneratorConfigPtr
DynamicElectrification::getOrCreateMeshConfig(std::string const& presetName)
{
    checkPresetFileUpdated(presetName);

    {
        std::lock_guard<std::mutex> lock(m_cachedMeshConfigsMutex);

        auto findIt = m_cachedMeshConfigs.find(presetName);
        if (findIt != m_cachedMeshConfigs.cend())
        {
            return findIt->second;
        }
        else
        {
            std::string presetPath = Engine::get()->getDataPath() + presetName;

            auto config = std::make_shared<ElectrificationAltMeshGeneratorConfig>();
            if (config->fillPropertiesFromJsonFile(presetPath))
            {
                m_cachedMeshConfigs.emplace(presetName, config);

                return config;
            }
        }
    }

    return nullptr;
}

ElectrificationNodeNameChooserPtr
DynamicElectrification::getOrCreateNodeNameChooser(std::string const& presetName)
{
    checkPresetFileUpdated(presetName);

    {
        std::lock_guard<std::mutex> lock(m_cachedNameChoosersMutex);

        auto findIt = m_cachedNameChoosers.find(presetName);
        if (findIt != m_cachedNameChoosers.cend())
        {
            return findIt->second;
        }
    }

    auto config = getOrCreateMeshConfig(presetName);
    if (!config)
    {
        return nullptr;
    }

    auto nodeNameChooser = std::make_shared<ElectrificationAltNodeNameChooser>(config);

    {
        std::lock_guard<std::mutex> lock(m_cachedNameChoosersMutex);
        m_cachedNameChoosers.emplace(presetName, nodeNameChooser);
    }

    return nodeNameChooser;
}

void DynamicElectrification::registerGenerated(DynamicElectrificationPartPtr const& generated)
{
    std::lock(m_registeredGeneratedMutex, m_generatedPromisesMutex);
    std::lock_guard<std::mutex> lock1(m_registeredGeneratedMutex, std::adopt_lock);
    std::lock_guard<std::mutex> lock2(m_generatedPromisesMutex, std::adopt_lock);

    auto partKey = generated->getKey();

    {
        auto findIt = m_registeredGenerated.find(partKey.key);
        if (findIt != m_registeredGenerated.cend())
        {
            auto const& oldGenerated = findIt->second;
            if (!generated->isDifferentWith(oldGenerated))
            {
                return;
            }
        }
    }

    m_registeredGenerated.emplace(partKey.key, generated);

    {
        auto findIt = m_generatedPromises.find(partKey.key);
        if (findIt != m_generatedPromises.cend())
        {
            DeferedGetGeneratedResult& item = findIt->second;
            GeneratedPromise& promise = item.first;
            promise.set_value(generated);
        }
    }
}

void DynamicElectrification::unregisterGenerated(uint64_t key)
{
    std::lock(m_registeredGeneratedMutex, m_generatedPromisesMutex);
    std::lock_guard<std::mutex> lock1(m_registeredGeneratedMutex, std::adopt_lock);
    std::lock_guard<std::mutex> lock2(m_generatedPromisesMutex, std::adopt_lock);

    {
        auto findIt = m_registeredGenerated.find(key);
        if (findIt != m_registeredGenerated.cend())
        {
            m_registeredGenerated.erase(key);
        }
    }

    {
        auto findIt = m_generatedPromises.find(key);
        if (findIt != m_generatedPromises.cend())
        {
            m_generatedPromises.erase(key);
        }
    }
}

DynamicElectrificationPartPtr DynamicElectrification::getGenerated(uint64_t key) const
{
    std::lock_guard<std::mutex> lock(m_registeredGeneratedMutex);

    auto findIt = m_registeredGenerated.find(key);
    if (findIt != m_registeredGenerated.cend())
    {
        return findIt->second;
    }

    return nullptr;
}

std::shared_future<DynamicElectrificationPartPtr>
DynamicElectrification::getGeneratedAsync(uint64_t key)
{
    std::lock(m_registeredGeneratedMutex, m_generatedPromisesMutex);
    std::lock_guard<std::mutex> lock1(m_registeredGeneratedMutex, std::adopt_lock);
    std::lock_guard<std::mutex> lock2(m_generatedPromisesMutex, std::adopt_lock);

    {
        auto findIt = m_generatedPromises.find(key);
        if (findIt != m_generatedPromises.cend())
        {
            DeferedGetGeneratedResult& item = findIt->second;
            return item.second;
        }
    }

    GeneratedPromise promise;
    GeneratedFuture future = promise.get_future().share();
    {
        auto findIt = m_registeredGenerated.find(key);
        if (findIt != m_registeredGenerated.cend())
        {
            promise.set_value(findIt->second);
        }
    }

    m_generatedPromises.emplace(key, std::make_pair(std::move(promise), future));

    return future;
}

bool DynamicElectrification::isPresetFileUpdated(std::string const& presetName) const
{
    namespace fs = std::filesystem;

    fs::path rootPath{ Unigine::Engine::get()->getDataPath() };
    fs::path fullPresetPath = rootPath / fs::path{ presetName };
    std::string key = fullPresetPath.generic_u8string();

    if (!fs::exists(fullPresetPath))
    {
        return false;
    }

    fs::file_time_type prevModifyTime;
    {
        std::lock_guard<std::mutex> lock(m_presetLastModifyTimesMutex);

        auto findIt = m_presetLastModifyTimes.find(key);
        if (findIt == m_presetLastModifyTimes.cend())
        {
            return true;
        }
    }

    fs::file_time_type currentModifyTime = fs::last_write_time(fullPresetPath);
    if (currentModifyTime != prevModifyTime)
    {
        return true;
    }

    return false;
}

void DynamicElectrification::updatePresetFileModifyTime(std::string const& presetName)
{
    namespace fs = std::filesystem;

    fs::path rootPath{ Unigine::Engine::get()->getDataPath() };
    fs::path fullPresetPath = rootPath / fs::path{ presetName };
    std::string key = fullPresetPath.generic_u8string();

    if (!fs::exists(fullPresetPath))
    {
        return;
    }

    fs::file_time_type currentModifyTime = fs::last_write_time(fullPresetPath);

    {
        std::lock_guard<std::mutex> lock(m_presetLastModifyTimesMutex);
        m_presetLastModifyTimes[key] = currentModifyTime;
    }
}

void DynamicElectrification::checkPresetFileUpdated(std::string const& presetName)
{
#ifndef DYNAMIC_ELECTRIFICATION_NO_FILE_UPDATE_TEST
    if (isPresetFileUpdated(presetName))
    {
        updatePresetFileModifyTime(presetName);

        std::lock(m_cachedMeshConfigsMutex, m_cachedNameChoosersMutex);
        std::lock_guard<std::mutex> lock1(m_cachedMeshConfigsMutex, std::adopt_lock);
        std::lock_guard<std::mutex> lock(m_cachedNameChoosersMutex, std::adopt_lock);

        m_cachedMeshConfigs.erase(presetName);
        m_cachedNameChoosers.erase(presetName);
    }
#endif
}


