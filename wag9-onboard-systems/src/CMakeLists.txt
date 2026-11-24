include_directories(. ${CMAKE_CURRENT_BINARY_DIR})

include(../cmake/compileProtoSources.cmake)
include(../cmake/GenerateProductVersion.cmake)
include(../cmake/RetriveGitHeadHash.cmake)

# Set -DDEVELOPER_MODE, but only in Debug configuration.
set_property(
    DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
    $<$<CONFIG:Debug>:DEVELOPER_MODE>
)

set(
    ui_files

    ../ui/Wag9Fs.ui
)
source_group("Sources" FILES ${ui_files})
qt5_wrap_ui(ui_sources ${ui_files})

source_group(
    "Generated" FILES
    ${ui_sources}
    REGULAR_EXPRESSION "(.*_automoc|mocs_compilation)\\.cpp"
)

set(
    neokon_shared_sources

    neokon-shared/TrainControls/Class170/Class170VarsToVirtualControlViewer.cpp
    neokon-shared/TrainControls/Class170/Class170VarsToVirtualControlViewer.h
)
source_group("Sources\\neokon-shared" FILES ${neokon_shared_sources})

set(
    neokon_shared_world_parse_sources

    neokon-shared/WorldParse/XmlTransformToGlmMatrix.cpp
    neokon-shared/WorldParse/XmlTransformToGlmMatrix.h
)
source_group("Sources\\neokon-shared\\WorldParse" FILES ${neokon_shared_world_parse_sources})

set(
    common_sources
    AutomatonButton.cpp AutomatonButton.h
    ControlButtonState.cpp ControlButtonState.h
    EventExecutor.cpp EventExecutor.h
    ExecEvent.cpp ExecEvent.h
    FileNames.cpp FileNames.h
    HitachiButton.cpp HitachiButton.h
    KeySwitchButton.cpp KeySwitchButton.h
    LogMessageHandler.cpp LogMessageHandler.h
    ScopeGuard.h
    UncheckableButtonGroup.cpp UncheckableButtonGroup.h
    XmlIO.cpp XmlIO.h
    ZmqBasicServer.cpp ZmqBasicServer.h
)
source_group("Sources" FILES ${common_sources})

add_library(
    neokon STATIC
    ${neokon_shared_sources}
    ${neokon_shared_world_parse_sources}
    ${neokon_shared_supply_change_marks_sources}
)

add_library(
    common STATIC
    ${common_sources} ${ui_sources}
#     ${ui_sources}
)

function (prepare_proto base_names_file group_name proto_files_out protobuf_generated_sources_out)

    include(${base_names_file}) # Defines the "proto_base_names" variable.
    set(proto_include_dirs
        ${CMAKE_CURRENT_SOURCE_DIR}/neokon-shared/proto
        ${CMAKE_CURRENT_SOURCE_DIR}/neokon-shared/proto/buzzers
        ${CMAKE_CURRENT_SOURCE_DIR}/neokon-shared/proto/crossing
        ${CMAKE_CURRENT_SOURCE_DIR}/neokon-shared/proto/electrification
        ${CMAKE_CURRENT_SOURCE_DIR}/neokon-shared/proto/meshgen
        ${CMAKE_CURRENT_SOURCE_DIR}/neokon-shared/proto/pneumatics
        ${CMAKE_CURRENT_SOURCE_DIR}/neokon-shared/proto/roadset)

    get_filename_component(input_dir ${base_names_file} DIRECTORY)
    set(proto_files_local "")
    set(protobuf_generated_sources_local "")

    compile_proto_files(
        proto_files_local
        protobuf_generated_sources_local
        ${input_dir}
        ${CMAKE_CURRENT_BINARY_DIR}
        "${proto_include_dirs}"
        ${proto_base_names}
    )

    set(tmp_var "")
    list(APPEND tmp_var ${${proto_files_out}})
    list(APPEND tmp_var ${proto_files_local})

    set(${proto_files_out} ${tmp_var} PARENT_SCOPE)

    set(tmp_var "")
    list(APPEND tmp_var ${${protobuf_generated_sources_out}})
    list(APPEND tmp_var ${protobuf_generated_sources_local})

    set(${protobuf_generated_sources_out} ${tmp_var} PARENT_SCOPE)

    source_group("proto\\${group_name}" FILES ${proto_files_local} ${base_names_file})
    source_group("Generated\\${group_name}" FILES ${protobuf_generated_sources_local})

endfunction()

set(proto_files "")
set(protobuf_generated_sources "")

PREPARE_PROTO(
    "${CMAKE_CURRENT_SOURCE_DIR}/neokon-shared/proto/base-names.cmake"
    "common"
    proto_files
    protobuf_generated_sources)

PREPARE_PROTO(
    "${CMAKE_CURRENT_SOURCE_DIR}/neokon-shared/proto/buzzers/base-names.cmake"
    "buzzers"
    proto_files
    protobuf_generated_sources)

PREPARE_PROTO(
    "${CMAKE_CURRENT_SOURCE_DIR}/neokon-shared/proto/crossing/base-names.cmake"
    "crossing"
    proto_files
    protobuf_generated_sources)

PREPARE_PROTO(
    "${CMAKE_CURRENT_SOURCE_DIR}/neokon-shared/proto/electrification/base-names.cmake"
    "electrification"
    proto_files
    protobuf_generated_sources)

PREPARE_PROTO(
    "${CMAKE_CURRENT_SOURCE_DIR}/neokon-shared/proto/meshgen/base-names.cmake"
    "meshgen"
    proto_files
    protobuf_generated_sources)

PREPARE_PROTO(
    "${CMAKE_CURRENT_SOURCE_DIR}/neokon-shared/proto/pneumatics/base-names.cmake"
    "pneumatics"
    proto_files
    protobuf_generated_sources)

PREPARE_PROTO(
    "${CMAKE_CURRENT_SOURCE_DIR}/neokon-shared/proto/roadset/base-names.cmake"
    "roadset"
    proto_files
    protobuf_generated_sources)

add_library(
    proto STATIC
    ${protobuf_generated_sources} ${proto_files} ${proto_base_names_file}
    ${CMAKE_SOURCE_DIR}/cmake/CompileProtoSources.cmake
)
source_group("Scripts" REGULAR_EXPRESSION ".cmake\$")

if(MSVC)
    target_compile_options(
        proto

        # disable warning C4065: switch statement contains 'default' but no 'case' labels"
        PRIVATE "-wd4065"
    )
endif()

target_link_libraries(
    proto
    protobuf::libprotobuf ZLIB::ZLIB
)

target_link_libraries(
    common
    proto neokon pugixml
    Qt5::Widgets Qt5::Xml Qt5::Sql Qt5::Network Qt5::Charts Qt5::Multimedia Qt5::Svg
    Boost::boost Boost::system Boost::filesystem
    glm nanoflann csv-parser libzmq-static cppzmq-static
    kainjow-mustache protobuf::libprotobuf ZLIB::ZLIB
    BugTrap
    ${CMAKE_DL_LIBS}
)

if(WIN32)
    find_library(PSAPI Psapi)
    target_link_libraries(common PSAPI)
endif ()

if(MSVC)
    set_source_files_properties(
        src/neokon-shared/BezierArcLengthCalculator.cpp
        COMPILE_FLAGS "/fp:fast" # Enable extra floating point optimizations.
    )
endif()

string(TIMESTAMP BUILD_COUNTER "%m%d")
string(TIMESTAMP BUILD_REVISION "%H%M")

set(COMPANY_NAME      "SMART Railway Training Simulators")
set(COMPANY_COPYRIGHT "SMART Simulators (c) 2023")

RetriveGitHeadHash()

generate_product_version(
    Wag9VersionFiles
    NAME "WAG9"
    ICON res/misc_icons/logo.ico
    VERSION_MAJOR 2
    VERSION_MINOR 0
    VERSION_PATCH ${BUILD_COUNTER}
    VERSION_REVISION ${BUILD_REVISION}
    COMPANY_NAME ${COMPANY_NAME}
    COMPANY_COPYRIGHT ${COMPANY_COPYRIGHT}
    OUTPUT_HEADER_FILE_NAME "Wag9Version.h"
    OUTPUT_RECOURCE_FILE_NAME "Wag9VersionResource.rc"
)


file(GLOB top_level_templates "../res/templates/*.mustache")
source_group("Templates" FILES ${top_level_templates})

file(GLOB driver_fault_templates "../res/templates/driver-faults/*.mustache")
source_group("Templates\\Driver Faults" FILES ${driver_fault_templates})

add_subdirectory(wag9)

target_link_libraries(neokon proto glm Boost::system Boost::filesystem nanoflann pugixml csv-parser)

set_target_properties(
    wag9_fs neokon
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT wag9_fs)
set_target_properties(neokon
    PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS true
)
