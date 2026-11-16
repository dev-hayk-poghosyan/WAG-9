#pragma once

#include "neokon-shared/IntervalPropertyMap.h"

#include <pugixml.hpp>

#include <QRect>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QHostAddress>

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <optional>

class Wag9Configuration
{
public:
    struct WindowSettings
    {
        QPoint position{ 0, 0 };
        int transitionMs = 50;
        bool debug = false;
        bool topMost = false;
        std::string bindAddress;
        QString audioOutputDevice;
    };

    struct NetworkAddress
    {
        QHostAddress address;
        unsigned short remotePort{};
        unsigned short localPort{};
    };

    struct ControllerSettings : public WindowSettings
    {
        NetworkAddress trainSimAddress;
        NetworkAddress radioStationAddress;
        NetworkAddress pultAddress;

        IntervalPropertyMap<int> reverserTranslateTable;
        IntervalPropertyMap<int> throttleTranslateTable;

        int deskVersion = 1;
    };

    /**
     * @throw std::exception If anything goes wrong.
     */
    explicit Wag9Configuration(pugi::xml_node rootNode);

    const WindowSettings& manometerSettings() const;
    const WindowSettings& speedmeterSettings() const;
    const WindowSettings& mcbSettings() const;
    const WindowSettings& fsSettings() const;
    const ControllerSettings& controllerSettings() const;

    const WindowSettings& pultScreen1Settings() const;
    const WindowSettings& pultScreen2Settings() const;
    const WindowSettings& pultScreen3Settings() const;

    const std::string& remoteAddress() const;
private:
    WindowSettings readWindowSettings(pugi::xml_node node) const;
    ControllerSettings readControllerSettings(pugi::xml_node node) const;

    WindowSettings m_manometerSettings;
    WindowSettings m_speedmeterSettings;
    WindowSettings m_mcbSettings;
    WindowSettings m_fsSettings;
    ControllerSettings m_controllerSettings;

    WindowSettings m_pultScreen1;
    WindowSettings m_pultScreen2;
    WindowSettings m_pultScreen3;

    std::string m_remoteAddress;
};

/**
 * Reads static configuration from an XML file.
 *
 * @throw std::exception If anything goes wrong.
 */
std::shared_ptr<Wag9Configuration const> loadWag9Configuration(QString const& xmlFilePath);
