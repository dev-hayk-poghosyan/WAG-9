#pragma once

/**
 * All possible signal states for UK Semaphore.
 *
 * @note The numeric values are saved into the scenario.
 */
enum class UkSemaphoreAspectState
{
    Off = 0,
    Closed = 1,
    Opened = 2,
    Causion = 3,
    Shunt = 4,
};
