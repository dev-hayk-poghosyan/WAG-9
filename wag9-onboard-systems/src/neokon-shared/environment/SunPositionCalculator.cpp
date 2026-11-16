#include "SunPositionCalculator.h"

#include "SunPosition.h"

#include <UnigineWorld.h>

#include <stdexcept>

using namespace Unigine;

SunPositionCalculator::SunPositionCalculator()
    : m_geodeticPivotNode(getGeodeticPivotNodeOrThrow())
{
}

SunPosition
SunPositionCalculator::calcSunPosition(
    int dayOfYear, float secondsSinceMidnight,
    Unigine::Math::Vec3 const& observerWorldPos) const
{
    GeodeticPos const observerGeodeticPos = worldPosToGeodeticPos(observerWorldPos);

    // The following calculations are based on:
    // https://astronomy.stackexchange.com/questions/1855/how-to-get-the-longitude-latitude-from-solar-zenith-azimuth
    //
    // Verification is done at https://www.suncalc.org/

    const float localTimeDifferenceGMT = 0.f; // Fixed for now.
    const float EclipticsAngle = 23.f + 26.f / 60.f; // 23 deg 26'

    float const B = 360.0f / 365.0f * (float(dayOfYear) - 81.f) * UNIGINE_DEG2RAD;
    float const SinB = Math::sin(B);
    float const equationOfTime = 9.87f * Math::sin(2.0f * B) - 7.53f * Math::cos(B) - 1.5f * SinB;
    float const sunDeclination = EclipticsAngle * UNIGINE_DEG2RAD * SinB;
    // alternate calcs are here: https://en.wikipedia.org/wiki/Position_of_the_Sun
    //const int N = dayNumber + 10; // days after December's solstice
    //m_sunDeclination = -EclipticsAngle * Math::cos(360.0 / 365.0 * N);

    float sinSunDeclination = 0.f;
    float cosSunDeclination = 0.f;
    Math::sincos(sunDeclination, sinSunDeclination, cosSunDeclination);

    // Time Correction factor, minutes - disabled for now because we use terrains from the 
    // different regions and cannot apply separate time meridian for each world.
    // float const localStandardTimeMeridian = 15.0f * localTimeDifferenceGMT;
    // float const TC = 4.0f * (float(observerGeodeticPos.longitude) - localStandardTimeMeridian)
    //                  + equationOfTime;
    const float TC = equationOfTime;
    // Local Solar Time, hours
    const float LST = secondsSinceMidnight / 3600.0f + TC / 60.0f;
    // Hour Angle
    const float HRA = 15.0f * (LST - 12.0f);
    const float CosHRA = Math::cos(HRA * UNIGINE_DEG2RAD);
    const float SinHRA = Math::sin(HRA * UNIGINE_DEG2RAD);
    // Elevation angle, radians
    const float LatitudeRadians = static_cast<float>(observerGeodeticPos.latitude) * UNIGINE_DEG2RAD;
    float cosLat = 1.f, sinLat = 0.f;
    Math::sincos(LatitudeRadians, sinLat, cosLat);
    const double SinElevation = sinSunDeclination * sinLat + cosSunDeclination * cosLat * CosHRA;
    const double ElevationAlpha = Math::asin(SinElevation);
    // Azimuth angle
    double cosElevation = Math::cos(ElevationAlpha);
    double azimuthAlpha = 0.0;
    double sinAzimuth = 0.0, cosAzimuth = 1.0;
    if (Math::abs(cosElevation) < UNIGINE_EPSILON)
    {
        cosElevation = UNIGINE_EPSILON * Math::sign(cosElevation);
    }
    cosAzimuth = (sinSunDeclination * cosLat - cosSunDeclination * sinLat * CosHRA) /
        cosElevation;
    sinAzimuth = -SinHRA * cosSunDeclination / cosElevation;

    azimuthAlpha = Math::atan2(sinAzimuth, cosAzimuth);

    // convert from azimuth/elevation to quaternion rotation in Unigine coordinates
    double azimuthAxisCorrected = 90.0 - azimuthAlpha * UNIGINE_RAD2DEG;
    Math::quat rotationLong(0.0f, 0.0f, azimuthAxisCorrected);
    Math::quat rotationLat(0.0f, (UNIGINE_PI05 - ElevationAlpha) * UNIGINE_RAD2DEG, 0.0f);

    // We don't have to transform this rotation from geodetic to world coordinates,
    // as the world X axis points along a parallel, and the Y axis - along a merridian.
    return SunPosition(rotationLong * rotationLat);
}

Unigine::GeodeticPivotPtr
SunPositionCalculator::getGeodeticPivotNodeOrThrow()
{
    Unigine::NodePtr node = Unigine::World::getNodeByName("geodetic_pivot");
    if (!node)
    {
        node = Unigine::World::getNodeByName("geodetic_pivot_flat");
    }

    auto typedNode = Unigine::checked_ptr_cast<Unigine::GeodeticPivot>(node);
    if (!typedNode)
    {
        throw std::logic_error("Unable to find the geodetic pivot node in the world");
    }

    return typedNode;
}

SunPositionCalculator::GeodeticPos
SunPositionCalculator::worldPosToGeodeticPos(Unigine::Math::Vec3 const& worldPos) const
{
    Math::Mat4 worldTransform;
    worldTransform.setTranslate(worldPos);

    Math::Vec3 const geodeticPos = m_geodeticPivotNode->toGeodetic(worldTransform);
    return GeodeticPos{ geodeticPos.x, geodeticPos.y };
}
