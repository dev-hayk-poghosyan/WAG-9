#pragma once

#include <QString>

enum class Wag9Side
{
    Left,
    Right,
};

enum class Wag9Zone
{
    Pantograph1,
    PT,
    SB1,
    HB1,
    Pantograph2,
    SB2,
    HB2,
};

QString toString(Wag9Side size);
QString toString(Wag9Zone zone);