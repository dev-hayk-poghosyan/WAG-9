#include "Wag9Configuration.h"

#include "XmlIO.h"

#include <QDebug>

Wag9Configuration::Wag9Configuration(pugi::xml_node rootNode)
{
    m_fsSettings = readWindowSettings(rootNode.child("FS"));

    const QString brakeSystem = QString::fromUtf8(
        rootNode.child("FS").child("BrakeSystem").text().as_string("CCB")
    ).trimmed().toUpper();

    if (brakeSystem == QLatin1String("E70"))
    {
        m_brakeSystem = BrakeSystem::E70;
    }

    else if (brakeSystem != QLatin1String("CCB"))
    {
        qWarning() << "Unknown BrakeSystem" << brakeSystem << "in config, using CCB";
    }
}

const Wag9Configuration::WindowSettings&
Wag9Configuration::fsSettings() const
{
    return m_fsSettings;
}

BrakeSystem
Wag9Configuration::brakeSystem() const
{
    return m_brakeSystem;
}

Wag9Configuration::WindowSettings
Wag9Configuration::readWindowSettings(pugi::xml_node node) const
{
    auto geometryNode = node.child("Geometry");
    return WindowSettings{ QPoint{
            geometryNode.attribute("left").as_int(0),
            geometryNode.attribute("top").as_int(0)},
        node.child("Debug").text().as_bool(),
        node.child("TopMost").text().as_bool(),
        node.child("Network").child("BindAddr").text().as_string(),
    };
}

std::shared_ptr<Wag9Configuration const>
loadWag9Configuration(QString const& xmlFilePath)
{
    qDebug() << "Loading a config file from" << xmlFilePath;

    pugi::xml_document doc = XmlIO::loadXML(xmlFilePath);
    return std::make_shared<Wag9Configuration const>(doc.first_child());
}
