#pragma once

#include <type_traits>

class UnitConv
{
public:
    constexpr static double kmh2ms(double kmh) {
        return kmh * (1.0 / 3.6);
    }

    constexpr static double ms2kmh(double ms) {
        return ms * 3.6;
    }

    constexpr static double ms2mph(double ms) {
        return ms * 2.23694;
    }

    constexpr static double mph2ms(double ms) {
        return ms * (1.0 / 2.23694);
    }

    /**
     * Converts meters to international mile. 
     * 1mile = 1 609.344 meters. Ergo 1meter = 1 / 1609.344 = 0.000621371 miles.
     */
    constexpr static double m2mile(double meters) {
        return meters * 0.000621371;
    }

    template<typename F>
    constexpr static F km2mile(F km)
    {
        static_assert(std::is_floating_point<F>::value, "Floating point type is required");
        return km * static_cast<F>(0.621371);
    }

    template<typename F>
    constexpr static F mile2km(F mile)
    {
        static_assert(std::is_floating_point<F>::value, "Floating point type is required");
        return mile * static_cast<F>(1.609344);
    }

    template<typename F>
    constexpr static F mile2m(F mile)
    {
        static_assert(std::is_floating_point<F>::value, "Floating point type is required");
        return mile * static_cast<F>(1609.344);
    }
};
