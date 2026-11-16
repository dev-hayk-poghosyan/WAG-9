set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS ON)

find_package(
    Boost 1.86 CONFIG REQUIRED COMPONENTS
    system thread atomic filesystem date_time regex program_options stacktrace_windbg)

add_definitions(-DBOOST_SYSTEM_USE_UTF8 -DBOOST_UUID_FORCE_AUTO_LINK)
