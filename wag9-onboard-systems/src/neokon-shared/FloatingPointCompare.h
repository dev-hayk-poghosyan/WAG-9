#pragma once

#include <limits>

template<typename T, int Precision = 0>
class TFloatingPointEpsilon
{
public:
    [[nodiscard]]
    static constexpr T epsilon() noexcept
    {
        if (Precision == 0)
        {
            return std::numeric_limits<T>::epsilon();
        }

        return negate(pow(T{ 10 }, Precision));
    }

    static T round(T value)
    {
        static const uint32_t delim = pow(uint32_t{ 10 }, Precision);

        return std::round(value * delim) / delim;
    }

private:
    static inline constexpr T pow(const T base, unsigned const exponent)
    {
        if (exponent > 0)
        {
            return base * pow(base, exponent - 1);
        }

        return 1;
    }

    static inline constexpr T negate(const T positiveValue)
    {
        return T{ 1 } / positiveValue;
    }
};

/**
 * @brief A class to correctly compare floating point values.
 */
template <typename T, int Precision = 0>
class TFloatingPointCompare : public TFloatingPointEpsilon<T, Precision>
{
public:
    /**
     * @a ~= @b (almost equal, e.i  @a += epsilon == @b
     */
    static bool equal(T a, T b)
    {
        return std::abs(a - b) < epsilon();
    }

    /**
     * @a >> @b (and @a not ~= @b)
     */
    static bool moreGreater(T a, T b)
    {
        return a > b && !equal(a, b);
    }

    /**
     * @a << @b (and @a not ~= @b)
     */
    static bool moreLess(T a, T b)
    {
        return a < b && !equal(a, b);
    }

    /**
     * @a > @b (or @a  ~= @b)
     */
    static bool greaterEqual(T a, T b)
    {
        return a > b || equal(a, b);
    }

    /**
     * @a < @b (or @a ~= @b)
     */
    static bool lessEqual(T a, T b)
    {
        return a < b || equal(a, b);
    }

    /**
     * clamps @value  [@lowerBound, @upperBound]
     */
    static T clampClosed(T value, T lowerBound, T upperBound)
    {
        if (greaterEqual(value, upperBound))
        {
            return upperBound;
        }

        if (lessEqual(value, lowerBound))
        {
            return lowerBound;
        }

        return value;
    }

    /**
     * clamps @value  (@lowerBound, @upperBound)
     */
    static T clampOpen(T value, T lowerBound, T upperBound)
    {
        if (greaterEqual(value, upperBound))
        {
            return (upperBound - epsilon());
        }

        if (lessEqual(value, lowerBound))
        {
            return (lowerBound + epsilon());
        }

        return value;
    }

    /**
     * @returns sign of floating point value or null with the given precision
     */
    static T sign(T value)
    {
        if (equal(value, T{ 0 }))
        {
            return 0;
        }
        return value < 0 ? -1 : 1;
    }
};

using DefaultFloatCompare = TFloatingPointCompare<float>; //Сравнение с наибольшей точностью.
using DefaultDoubleCompare = TFloatingPointCompare<double>;
using CmFloatCompare = TFloatingPointCompare<float, 3>; //Сравнение с точностью до сантиметра.
using CmDoubleCompare = TFloatingPointCompare<double, 3>;
using MmFloatCompare = TFloatingPointCompare<float, 4>; //Сравнение с точностью до миллиметра.
using MmDoubleCompare = TFloatingPointCompare<double, 4>;
