find_package(ZeroMQ 4.0 CONFIG REQUIRED)

# Unfortunately, the CMake targets for ZeroMQ don't define ZMQ_BUILD_DRAFT_API
# even if draft APIs were explicitly enabled at configure time. Therefore, we do that manually.
add_definitions(-DZMQ_BUILD_DRAFT_API)
