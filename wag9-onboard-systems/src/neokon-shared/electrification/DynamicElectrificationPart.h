#pragma once

#include "neokon-shared/railways/DynamicRailwayPart.h"

#ifdef DYNAMIC_ELECTRIFICATION_DEBUG
#include "neokon-shared/railways/ObjectDebugVisualizer.h"
#include "neokon-shared/railways/ObjectMessageLogger.h"
#endif

class DynamicElectrificationPart;
using DynamicElectrificationPartPtr = std::shared_ptr<DynamicElectrificationPart>;

class DynamicElectrificationPart : public DynamicRailwayPart
{
public:
    /**
     * @returns std::nullopt if the node couldn't have been loaded.
     *
     * @note: must be called from the main thread only since it temporarily loads Unigine nodes.
     */
    static std::optional<Unigine::Math::Mat4> getOrLoadInitialNodeTransform(
        const std::string& path);

    DynamicElectrificationPart(uint32_t id, uint32_t subId);
    ~DynamicElectrificationPart() = default;

    virtual void unload() override;

    bool isLoaded() const {  return m_isLoaded; }

public: // Debugging
#ifdef DYNAMIC_ELECTRIFICATION_DEBUG
    void setDebugLogger(const ObjectMessageLoggerPtr& logger);
    void setDebugVisualizer(const ObjectDebugVisualizerPtr& visualizer);
#endif

protected:
    void onLoadComplete();

    /**
     * Function adds debug message to this part's own log, placed here for possibility to disable
     * logging without code changes. If 'setDebugLogger' was set for this part, messages will be 
     * added to it, if not - they will be dropped. If DYNAMIC_RAILWAYS_DEBUG is not defined,
     * function will do nothing and will be removed by compiler from release build.
     */
    void message(const char* format, ...) const;

    /**
     * Part's log warning message. See documentation to @p message for details.
     */
    void warning(const char* format, ...) const;

    /**
     * Part's log error message. See documentation to @p message for details.
     */
    void error(const char* format, ...) const;

private:
    std::atomic_bool m_isLoaded{ false };

#ifdef DYNAMIC_ELECTRIFICATION_DEBUG
    ObjectMessageLoggerPtr m_debugLogger;
    ObjectDebugVisualizerPtr m_debugVisualizer;
#endif
};
