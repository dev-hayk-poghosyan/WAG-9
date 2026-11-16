#include "Direction.h"

Direction reverseDirection(Direction dir)
{
    return dir == Direction::Forward ? Direction::Backward : Direction::Forward;
}
