#pragma once

#include "DynamicRailwayPart.h"

#ifdef DYNAMIC_RAILWAYS_DEBUG
#include "ObjectDebugVisualizer.h"
#include "ObjectMessageLogger.h"
#endif

#include "neokon-shared/railways/RailwayMeshGeneratorConfig.h"

/**
 * This class is used by 'DynamicRailways' to generate nearby railway objects in runtime, it is
 * inherited from DynamicRailwayPart to have all the 'RailwayMeshGeneratorConfig' loading logic.
 */
class DynamicRailwayPartGenerated : public DynamicRailwayPart
{
public:
    static RailwayMeshGeneratorConfigPtr getOrLoadRailwayMeshGeneratorConfig(const std::string& path);
    DynamicRailwayPartGenerated(uint32_t id, uint32_t subId, const std::string& configPath,
        const RailwayMeshGeneratorConfigPtr& config = nullptr);
    virtual bool isDifferentWith(const DynamicRailwayPartPtr& old) const override;

public: // Debugging
#ifdef DYNAMIC_RAILWAYS_DEBUG
    void setDebugLogger(const ObjectMessageLoggerPtr& logger);
    void setDebugVisualizer(const ObjectDebugVisualizerPtr& visualizer);
#endif

protected:
    bool loadConfig();
    bool hasDifferentEmbankmentProfile(const RailwayMeshGeneratorConfigPtr& otherConfig);

protected:
    /**
     * Function adds debug message to this part's own log, placed here for possibility to disable
     * logging without code changes. If 'setDebugLogger' was set for this part, messages will be 
     * added to it, if not - they will be dropped. If DYNAMIC_RAILWAYS_DEBUG is not defined,
     * function will do nothing and will be removed by compiler from release build.
     */
    void message(const char* format, ...);

    /**
     * Part's log warning message. See documentation to @p message for details.
     */
    void warning(const char* format, ...);

    /**
     * Part's log error message. See documentation to @p message for details.
     */
    void error(const char* format, ...);

protected:
    const std::string m_configPath;
    RailwayMeshGeneratorConfigPtr m_config;

#ifdef DYNAMIC_RAILWAYS_DEBUG
    ObjectMessageLoggerPtr m_debugLogger;
    ObjectDebugVisualizerPtr m_debugVisualizer;
#endif
};
