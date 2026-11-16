#pragma once

/**
 * Types of sound messages at the crossing.
 * Used when playing sound messages for student controlled train on rail crossing.
 */
enum class RuCrossingSoundType
{
    NoSound =                          0,
    SkidInTheBeginEven =               1,
    SkidInTheEndEven =                 2,
    SkidInTheMiddleEven =              3,
    SkidInTheBeginOdd =                4,
    SkidInTheEndOdd =                  5,
    SkidInTheMiddleOdd =               6,
    FireInTheBeginEven =               7,
    FireInTheEndEven =                 8,
    FireInTheMiddleEven =              9,
    FireInTheBeginOdd =                10,
    FireInTheEndOdd =                  11,
    FireInTheMiddleOdd =               12,
    TrainHello =                       13,
    EvenTrainOk =                      14,
    OddTrainOk =                       15,
    CrossingOpenWire =                 16,
    CrossingOpenWireEven =             17,
    CrossingOpenWireOdd =              18,
    CrossingOpenWireFixed =            19,
    CrossingCar =                      20,
    CrossingCarBoth =                  21,
    CrossingCarEven =                  22,
    CrossingCarOdd =                   23,
    CrossingCarFixed =                 24,
    DutyNames =                        25
};
