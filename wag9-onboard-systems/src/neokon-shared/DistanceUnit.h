#pragma once

#include <string>
#include <string_view>

struct DistanceUnit
{
public:
    static DistanceUnit m() { return DistanceUnit("m", 1.0, 1.0); }

    static DistanceUnit km() { return DistanceUnit("km", 1000.0, 1.0 / 1000.0); }

    static DistanceUnit mi() { return DistanceUnit("mi", 1609.344, 1.0 / 1609.344); }

    static DistanceUnit ft() { return DistanceUnit("ft", 0.3048, 1.0 / 0.3048); }

    DistanceUnit(std::string_view symbol, double metersInUnit, double unitsInMeter)
        : m_symbol(symbol)
        , m_metersInUnit(metersInUnit)
        , m_unitsInMeter(unitsInMeter)
    {}

    std::string const& symbol() const { return m_symbol; }

    double metersInUnit() const { return m_metersInUnit; }

    double unitsInMeter() const { return m_unitsInMeter; }

    double toMeters(double value) const { return value * m_metersInUnit; }

    double fromMeters(double value) const { return value * m_unitsInMeter; }

private:
    std::string m_symbol;
    double m_metersInUnit = 0.0;
    double m_unitsInMeter = 0.0;
};
