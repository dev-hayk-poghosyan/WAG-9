#include "DynamicElectrificationPart.h"

#include "DynamicElectrification.h"

#include <UnigineWorld.h>

namespace
{
// Cache for all nodes transforms applied in the .node file.
std::unordered_map<std::string, Unigine::Math::Mat4> transformCache;

} // Anonymous namespace

std::optional<Unigine::Math::Mat4>
DynamicElectrificationPart::getOrLoadInitialNodeTransform(const std::string& path)
{
    if (path.empty())
    {
        return std::nullopt;
    }

    auto transformIt = transformCache.find(path);
    if (transformIt != transformCache.end())
    {
        return transformIt->second;
    }

    if (auto loadedNode = Unigine::World::loadNode(path.c_str()))
    {
        Unigine::Math::Mat4 const nodeTransform(loadedNode->getTransform());
        loadedNode.deleteLater();
        transformCache.emplace(path, nodeTransform);

        return nodeTransform;
    }

    return std::nullopt;
}

DynamicElectrificationPart::DynamicElectrificationPart(uint32_t id, uint32_t subId)
    : DynamicRailwayPart(id, subId)
{
}

void DynamicElectrificationPart::unload()
{
    //TODO: данный код приводит к крашу приложения при закрытии.
    /*
    auto generatedHost = DynamicElectrification::instance();
    assert(generatedHost);
    generatedHost->unregisterGenerated(getKey().key);
    */

    m_isLoaded = false;
}

void DynamicElectrificationPart::onLoadComplete()
{
    m_isLoaded = true;

    auto generatedHost = DynamicElectrification::instance();
    assert(generatedHost);

    auto myPtr = std::dynamic_pointer_cast<DynamicElectrificationPart>(shared_from_this());
    generatedHost->registerGenerated(myPtr);
}

// Debugging

#ifdef DYNAMIC_ELECTRIFICATION_DEBUG
void DynamicElectrificationPart::setDebugLogger(const ObjectMessageLoggerPtr& logger)
{
    m_debugLogger = logger;
}

void DynamicElectrificationPart::setDebugVisualizer(const ObjectDebugVisualizerPtr& visualizer)
{
    m_debugVisualizer = visualizer;
}
#endif

void DynamicElectrificationPart::message(const char* format, ...) const
{
#ifdef DYNAMIC_ELECTRIFICATION_DEBUG
    if (m_debugLogger)
    {
        va_list argptr;
        va_start(argptr, format);
        m_debugLogger->debug(format, argptr);
        va_end(argptr);
    }
#endif
}

void DynamicElectrificationPart::warning(const char* format, ...) const
{
#ifdef DYNAMIC_ELECTRIFICATION_DEBUG
    if (m_debugLogger)
    {
        va_list argptr;
        va_start(argptr, format);
        m_debugLogger->warning(format, argptr);
        va_end(argptr);
    }
#endif
}

void DynamicElectrificationPart::error(const char* format, ...) const
{
#ifdef DYNAMIC_ELECTRIFICATION_DEBUG
    if (m_debugLogger)
    {
        va_list argptr;
        va_start(argptr, format);
        m_debugLogger->error(format, argptr);
        va_end(argptr);
    }
#endif
}
