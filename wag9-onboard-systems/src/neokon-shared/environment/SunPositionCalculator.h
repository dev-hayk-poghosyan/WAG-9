#pragma once

#include <UnigineMathLib.h>
#include <UnigineGeodetics.h>

class SunPosition;

class SunPositionCalculator
{
public:
    /**
     * @throw std::exception If anything goes wrong.
     */
    SunPositionCalculator();

    SunPosition calcSunPosition(
        int dayOfYear, float secondsSinceMidnight,
        Unigine::Math::Vec3 const& observerWorldPos) const;

private:
    struct GeodeticPos
    {
        double latitude;
        double longitude;
    };

    static Unigine::GeodeticPivotPtr getGeodeticPivotNodeOrThrow();

    GeodeticPos worldPosToGeodeticPos(Unigine::Math::Vec3 const& worldPos) const;

    Unigine::GeodeticPivotPtr m_geodeticPivotNode;
};
