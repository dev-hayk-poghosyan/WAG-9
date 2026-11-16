#include "Wag9Configuration.h"

#include "XmlIO.h"
#include <pugixml.hpp>


Wag9Configuration::Wag9Configuration(pugi::xml_node rootNode)
{
    const auto panelsNode = rootNode.child("Panels");
    m_manometerSettings = readWindowSettings(panelsNode.child("Manometer"));
    m_speedmeterSettings = readWindowSettings(panelsNode.child("Speedmeter"));
    m_mcbSettings = readWindowSettings(panelsNode.child("MCB"));
    m_controllerSettings = readControllerSettings(rootNode.child("Controller"));

    m_pultScreen1 = readWindowSettings(panelsNode.child("PultScreen1"));
    m_pultScreen2 = readWindowSettings(panelsNode.child("PultScreen2"));
    m_pultScreen3 = readWindowSettings(panelsNode.child("PultScreen3"));

    m_remoteAddress = panelsNode.child("Network").child("ConnectAddr").text().as_string();

    m_fsSettings = readWindowSettings(rootNode.child("FS"));
}

const Wag9Configuration::WindowSettings&
Wag9Configuration::manometerSettings() const
{
    return m_manometerSettings;
}

const Wag9Configuration::WindowSettings&
Wag9Configuration::speedmeterSettings() const
{
    return m_speedmeterSettings;
}

const Wag9Configuration::WindowSettings&
Wag9Configuration::mcbSettings() const
{
    return m_mcbSettings;
}

const Wag9Configuration::WindowSettings&
Wag9Configuration::fsSettings() const
{
    return m_fsSettings;
}

const Wag9Configuration::ControllerSettings&
Wag9Configuration::controllerSettings() const
{
    return m_controllerSettings;
}

const Wag9Configuration::WindowSettings&
Wag9Configuration::pultScreen1Settings() const
{
    return m_pultScreen1;
}

const Wag9Configuration::WindowSettings&
Wag9Configuration::pultScreen2Settings() const
{
    return m_pultScreen2;
}

const Wag9Configuration::WindowSettings&
Wag9Configuration::pultScreen3Settings() const
{
    return m_pultScreen3;
}

const std::string&
Wag9Configuration::remoteAddress() const
{
    return m_remoteAddress;
}

Wag9Configuration::WindowSettings
Wag9Configuration::readWindowSettings(pugi::xml_node node) const
{
    auto geometryNode = node.child("Geometry");
    return WindowSettings{ QPoint{
            geometryNode.attribute("left").as_int(0),
            geometryNode.attribute("top").as_int(0)},
        node.child("TransitionMs").text().as_int(0),
        node.child("Debug").text().as_bool(),
        node.child("TopMost").text().as_bool(),
        node.child("Network").child("BindAddr").text().as_string(),
        node.child("AudionOutput").text().as_string(),
    };
}

Wag9Configuration::ControllerSettings
Wag9Configuration::readControllerSettings(pugi::xml_node node) const
{
    auto readNetworkAddress = [](pugi::xml_node node)
    {
        Wag9Configuration::NetworkAddress address;
        address.address = QHostAddress(node.attribute("ip").as_string());
        address.remotePort = node.attribute("send_port").as_uint();
        address.localPort = node.attribute("bind_port").as_uint();
        return address;
    };

    auto geometryNode = node.child("Geometry");
    auto netNode = node.child("Network");

    ControllerSettings settings;
    settings.position = QPoint{ geometryNode.attribute("left").as_int(0),
            geometryNode.attribute("top").as_int(0) };
    settings.transitionMs = node.child("TransitionMs").text().as_int(0);
    settings.debug = node.child("Debug").text().as_bool();
    settings.topMost = node.child("TopMost").text().as_bool();
    settings.pultAddress = readNetworkAddress(netNode.child("Pult"));
    settings.trainSimAddress = readNetworkAddress(netNode.child("TrainSim"));
    settings.radioStationAddress = readNetworkAddress(netNode.child("RadioStation"));

    const auto& nativeNode = node.child("NativeController");

    const auto& controllerItems = nativeNode.child("ThrottleControllerData");
    qDebug("Map for throttle controller:");
    for (const auto& throttleNode : controllerItems.children("Level"))
    {
        const auto val = throttleNode.text().as_int();
        const auto from = throttleNode.attribute("from").as_int();
        const auto to = throttleNode.attribute("to").as_int();

        settings.throttleTranslateTable.setPropertyOnInterval(val, from, to);
        qDebug("\tfrom %d to %d = %d", from, to, val);
    }

    const auto& reverserItems = nativeNode.child("ReverserControllerData");
    qDebug("Map for reverser controller:");
    for (const auto& reverseNode : reverserItems.children("Level"))
    {
        const auto val = reverseNode.text().as_int();
        const auto from = reverseNode.attribute("from").as_int();
        const auto to = reverseNode.attribute("to").as_int();

        settings.reverserTranslateTable.setPropertyOnInterval(val, from, to);
        qDebug("\tfrom %d to %d = %d", from, to, val);
    }

    settings.deskVersion = nativeNode.child("Version").text().as_int(settings.deskVersion);
    qDebug("Native desk version %d", settings.deskVersion);

    return settings;
}

std::shared_ptr<Wag9Configuration const>
loadWag9Configuration(QString const& xmlFilePath)
{
    qDebug() << "Loading a config file from" << xmlFilePath;

    pugi::xml_document doc = XmlIO::loadXML(xmlFilePath);
    std::shared_ptr<Wag9Configuration const> result;

    return std::make_shared<Wag9Configuration const>(doc.first_child());
}
