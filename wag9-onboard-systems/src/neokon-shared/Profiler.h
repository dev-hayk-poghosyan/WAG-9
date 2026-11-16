#pragma once

#include <windows.h>

#ifdef ENABLE_PROFILER_TRACE
#define PROFILE_BUILD
#endif

#ifdef PROFILE_BUILD

#include <pix3.h>

#define PROFILER_FUNCTION                                                                          \
    PIXScopedEventObject<void> neoProf##__LINE__(PIX_COLOR(20, 200, 20), "%s", ##__FUNCTION__)

#define PROFILER_SCOPE(name)                                                                       \
    PIXScopedEventObject<void> neoProf##__LINE__(PIX_COLOR(20, 200, 20), "%s", name)

#define PROFILER_FMT(fmt, ...)                                                                     \
    PIXScopedEventObject<void> neoProf##__LINE__(PIX_COLOR(20, 200, 20), fmt, __VA_ARGS__)

#else

// Disabling profiler traces
// Direct PIX trace calls will work in _DEBUG anyway.

#define PROFILER_FUNCTION ;
#define PROFILER_SCOPE(name) ;
#define PROFILER_FMT(fmt, ...) ;

#endif

#define NEO_PROFILER_FUNCTION PROFILER_FUNCTION
#define NEO_PROFILER_FMT PROFILER_FMT
