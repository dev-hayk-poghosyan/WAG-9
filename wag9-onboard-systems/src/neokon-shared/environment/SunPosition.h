#pragma once

#include "TimeOfDay.h"

#include <UnigineMathLib.h>

class SunPosition
{
public:
    explicit SunPosition(Unigine::Math::quat const& sunRotation) : m_sunRotation(sunRotation) {}

    void applyToWorld() const;

    /**
     * Returns the sun altitude angle (aka the sun elevation angle) in degrees.
     *
     * The returned angle shall be within the range of [-90, 90] degrees.
     * Positive angles correspond to the sun being above the horizon.
     */
    float sunAltitudeAngleDeg() const;

    /**
     * Returns TimeOfDay::Day if the sun is high enough, and TimeOfDay::Night otherwise.
     */
    TimeOfDay timeOfDay() const;

private:
    /**
     * The rotation to give to the Unigine's Sun node.
     *
     * The identity rotation corresponds to the sun being in zenith.
     */
    Unigine::Math::quat m_sunRotation;
};
