#include "SunPosition.h"

#include <UnigineNode.h>
#include <UnigineWorld.h>

void
SunPosition::applyToWorld() const
{
    if (Unigine::NodePtr sunNode = Unigine::World::getNodeByName("sun"))
    {
        sunNode->setRotation(m_sunRotation);
    }
}

float
SunPosition::sunAltitudeAngleDeg() const
{
    using namespace Unigine;

    Math::vec3 const dirToSun = m_sunRotation * Math::vec3_up;

    float const dirToSunHorComponent = Math::length(Math::vec2(dirToSun.x, dirToSun.y));
    float const dirToSunVertComponent = dirToSun.z;

    return Math::atan2(dirToSunVertComponent, dirToSunHorComponent) * UNIGINE_RAD2DEG;
}

TimeOfDay
SunPosition::timeOfDay() const
{
    return sunAltitudeAngleDeg() < 8.f ? TimeOfDay::Night : TimeOfDay::Day;
}
