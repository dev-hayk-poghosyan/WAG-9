#pragma once

#include "neokon-shared/railways/DynamicRailways.h"
#include "neokon-shared/electrification/DynamicElectrificationPart.h"
#include "neokon-shared/electrification/ElectrificationAltNodeNameChooser.h"

#ifdef DYNAMIC_ELECTRIFICATION_DEBUG
#include "neokon-shared/railways/ObjectDebugVisualizer.h"
#include "neokon-shared/railways/ObjectMessageLogger.h"
#endif

#include <future>
#include <filesystem>

using DynamicElectrificationParts = std::vector<DynamicElectrificationPartPtr>;
using DynamicElectrificationGetDataFunc = std::function<DynamicElectrificationParts(const Unigine::Math::Vec3& center)>;

class DynamicElectrification : public DynamicRailways
{
public:
    static void registerClass();
    static DynamicElectrification* create(DynamicElectrificationGetDataFunc const& getDataFunc);
    static DynamicElectrification* instance();

    DynamicElectrification(void* object); // This constructor is used by Unigine internal deserialization
    DynamicElectrification(const DynamicRailwaysGetDataFunc& getDataFunc); // This is used by create(...)
    virtual ~DynamicElectrification();

    virtual int getClassID() override { return +ObjectExternIds::DynamicElectrification; }
    virtual void update(float ifps) override;

    ElectrificationMeshGeneratorConfigPtr getOrCreateMeshConfig(std::string const& presetName);
    ElectrificationNodeNameChooserPtr getOrCreateNodeNameChooser(std::string const& presetName);

    /**
     * Returns nullptr if generated with *key* not found,
     * or this one is not loaded yet
     */
    DynamicElectrificationPartPtr getGenerated(uint64_t key) const;

    /**
     * Returns future<> object of future DynamicElectrificationPart then it will be loaded
     * and registered.
     */
    std::shared_future<DynamicElectrificationPartPtr> getGeneratedAsync(uint64_t key);

#ifdef DYNAMIC_ELECTRIFICATION_DEBUG
    ObjectMessageLoggerPtr getDebugLogger() const { return m_debugLogger; }
    ObjectDebugVisualizerPtr getDebugVisualiser() const { return m_debugVisualizer; }
#endif

private:
    static DynamicElectrification* s_instance;

private:
    friend class DynamicElectrificationPart;

    using GeneratedPromise = std::promise<DynamicElectrificationPartPtr>;
    using GeneratedFuture = std::shared_future<DynamicElectrificationPartPtr>;
    using DeferedGetGeneratedResult = std::pair<GeneratedPromise, GeneratedFuture>;
    using FileModifyTime = std::filesystem::file_time_type;

    //call by DynamicElectrificationPart
    void registerGenerated(DynamicElectrificationPartPtr const& generated);
    void unregisterGenerated(uint64_t key);
    bool isPresetFileUpdated(std::string const& presetName) const;
    void updatePresetFileModifyTime(std::string const& presetName);
    void checkPresetFileUpdated(std::string const& presetName);

    std::unordered_map<std::string,
        ElectrificationMeshGeneratorConfigPtr> m_cachedMeshConfigs;
    std::mutex m_cachedMeshConfigsMutex;

    std::unordered_map<std::string,
        ElectrificationNodeNameChooserPtr> m_cachedNameChoosers;
    std::mutex m_cachedNameChoosersMutex;

    std::unordered_map<std::string, FileModifyTime> m_presetLastModifyTimes;
    mutable std::mutex m_presetLastModifyTimesMutex;

    std::unordered_map<uint64_t, DynamicElectrificationPartPtr> m_registeredGenerated;
    mutable std::mutex m_registeredGeneratedMutex;

    std::unordered_map<uint64_t, DeferedGetGeneratedResult> m_generatedPromises;
    std::mutex m_generatedPromisesMutex;

#ifdef DYNAMIC_ELECTRIFICATION_DEBUG
    ObjectMessageLoggerPtr m_debugLogger;
    ObjectDebugVisualizerPtr m_debugVisualizer;
#endif
};
