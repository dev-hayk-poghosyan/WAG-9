#include "DefaultThreadLocalRng.h"

std::default_random_engine& defaultThreadLocalRng()
{
    thread_local std::default_random_engine rng{
        (unsigned)std::chrono::high_resolution_clock::now().time_since_epoch().count()
    };

    return rng;
}
