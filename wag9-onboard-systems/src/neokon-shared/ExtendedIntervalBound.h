#pragma once

/**
 * Intervals may be open, closed or semi-closed. In other words, interval bounds
 * may be inclusive (denoted "[" or "]") or exclusive (denoted "(" or ")").
 * Here we introduce another type of interval bound: the extra-inclusive one.
 * Let's denote it as "+[" and "]+". It works by including one extra element
 * in the sub-range, if the boundary element is not present in the original range.
 * For example, if our sequence is "1, 3, 5, 7" and we take the interval of
 * "+[2, 4]+", then the resulting sequence will be "1, 3, 5", but "+[3, 5]+"
 * will give "3, 5", as those elements were present.
 */
enum class ExtendedIntervalBound
{
    Exclusive,
    Inclusive,
    ExtraInclusive
};
