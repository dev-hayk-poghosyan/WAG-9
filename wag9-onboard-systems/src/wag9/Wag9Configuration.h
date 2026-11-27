#pragma once

#include <pugixml.hpp>

#include <QPoint>
#include <QString>

#include <memory>
#include <string>

enum class BrakeSystem
{
    Ccb,
    E70
};

class Wag9Configuration
{
public:
    struct WindowSettings
    {
        QPoint position{ 0, 0 };
        bool debug = false;
        bool topMost = false;
        std::string bindAddress;
    };

    /**
     * @throw std::exception If anything goes wrong.
     */
    explicit Wag9Configuration(pugi::xml_node rootNode);

    const WindowSettings& fsSettings() const;
    BrakeSystem brakeSystem() const;

private:
    WindowSettings readWindowSettings(pugi::xml_node node) const;

    WindowSettings m_fsSettings;
    BrakeSystem m_brakeSystem = BrakeSystem::Ccb;
};

/**
 * Reads static configuration from an XML file.
 *
 * @throw std::exception If anything goes wrong.
 */
std::shared_ptr<Wag9Configuration const> loadWag9Configuration(QString const& xmlFilePath);
