find_package(Git)

find_program(
    PATCH_EXECUTABLE patch
    HINTS "${GIT_EXECUTABLE}/../../usr/bin"
    DOC "Full path to the patch executable"
)

if (NOT PATCH_EXECUTABLE)
    message(
        FATAL_ERROR
        "Unable to find the patch program. On Windows, the recommended version is the one "
        "that ships with the Git distribution. You may need to set the PATCH_EXECUTABLE "
        "variable manually to point to the path executable."
    )
endif()

# This command is intended to be used with the ExternalProject_Add() command.
# It generates a series of PATCH_COMMAND / COMMAND sequences to be passed to ExternalProject_Add().
# 
# Usage:
# generate_patch_commands(
#     <output var> [USE_GIT_AM] [PRE_COMMANDS ...]
#     PATCHES /path/to/patches/*.patch [more patches])
#
# The PRE_COMMANDS sequence will be prependend to the generated commands and is expected to start with
# "PATCH_COMMANDS".
#
function(generate_patch_commands out_var)

    cmake_parse_arguments(ARG "USE_GIT_AM" "" "PRE_COMMANDS;PATCHES" ${ARGN})

    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "generate_patch_commands(): unexpected arguments encountered")
    endif()

    set(commands ${ARG_PRE_COMMANDS})

    foreach(globbing_expr ${ARG_PATCHES})
        file(GLOB files LIST_DIRECTORIES false ${globbing_expr})
        list(SORT files)
        foreach(file ${files})
            if(commands)
                set(cmd "COMMAND")
            else()
                set(cmd "PATCH_COMMAND")
            endif()

            if(ARG_USE_GIT_AM)
                list(APPEND commands ${cmd} ${GIT_EXECUTABLE} am --3way ${file})
            else()
                list(APPEND commands ${cmd} ${PATCH_EXECUTABLE} -p1 -i ${file})
            endif()
        endforeach()
    endforeach()

    set(${out_var} ${commands} PARENT_SCOPE)
endfunction()
