#
# create_copy_files_target(
#    <target_name>
#    DESTINATION <dir>
#        CONFIGURATIONS <names|ALL> FILES <files>
#        [CONFIGURATIONS ...]
#    [DESTINATION ...]
# )
#
# Creates a target to copy a set of files into one or more directories
# that may be build-configuration specific.
#
# Configuration names are Debug, Release, etc. The destination directories may
# contain generator expressions, possibly making them configuration-specific.
# The file names can't contain generator expressions.
#
function(create_copy_files_target target_name)

    set(num_destinations 0)

    foreach(item ${ARGN})
        if(item STREQUAL "DESTINATION")
            math(EXPR num_destinations "${num_destinations} + 1")
            set(destination_${num_destinations} "")
        else()
            if(num_destinations EQUAL 0)
                message(FATAL_ERROR
                    "create_copy_files_target(): Unexpected arguments prior to DESTINATION")
            else()
                list(APPEND destination_${num_destinations} ${item})
            endif()
        endif()
    endforeach()

    set(commands "")
    set(file_deps "")

    foreach(dest_idx RANGE 1 ${num_destinations})
        set(destination_spec ${destination_${dest_idx}})

        cmake_parse_arguments(CONF "" "" "CONFIGURATIONS" ${destination_spec})

        list(LENGTH CONF_UNPARSED_ARGUMENTS num_dirs)
        if(num_dirs EQUAL 0)
            message(FATAL_ERROR "Missing directory name for DESTINATION #${dest_idx}")
        elseif(num_dirs GREATER 1)
            message(FATAL_ERROR "More than one directory name in DESTINATION #${dest_idx}")
        endif()

        # Extract dest_dir from destination_spec.
        list(GET destination_spec 0 dest_dir)

        # A variable for substitution by configure_file()
        set(CONFIGURATIONS_SPEC ${destination_spec})
        list(REMOVE_AT CONFIGURATIONS_SPEC 0) # Remove the dest_dir component.

        # Full path to the generated copy script.
        set(copy_script "${CMAKE_CURRENT_BINARY_DIR}/copy_files_${dest_idx}.cmake")

        # Add the generated copy script to dependencies.
        list(APPEND file_deps "${copy_script}")

        # Generate the copy script.
        configure_file("${COPY_FILES_SCRIPT_TEMPLATE}" "${copy_script}" @ONLY)

        # Add script invocation to the list of commands. Note that dest_dir may contain
        # generator expressions, which is why it has to be passed as a runtime argument.
        list(
            APPEND commands
            COMMAND "${CMAKE_COMMAND}"
                "-DCFG=$<CONFIG>" "-DDEST_DIR=${dest_dir}" -P "${copy_script}"
        )

        # In order to add dependencies on files to be copied, we need to parse
        # CONFIGURATIONS_SPEC to extract the list of files for every build configuration.

        set(num_configurations 0)

        foreach(item ${CONFIGURATIONS_SPEC})
            if(item STREQUAL "CONFIGURATIONS")
                math(EXPR num_configurations "${num_configurations} + 1")
                set(configuration_${num_configurations} "")
            else()
                list(APPEND configuration_${num_configurations} ${item})
            endif()
        endforeach(item)

        foreach(cfg_idx RANGE 1 ${num_configurations})

            cmake_parse_arguments(CONF "" "" "FILES" ${configuration_${cfg_idx}})
            foreach(cfg ${CONF_UNPARSED_ARGUMENTS})
                list(APPEND "FILES_${cfg}" ${CONF_FILES})
            endforeach(cfg)

        endforeach(cfg_idx)

        # Add dependencies on files to be copied.
        list(APPEND file_deps ${FILES_ALL})
        foreach(cfg ${CMAKE_CONFIGURATION_TYPES})
            foreach(file ${FILES_${cfg}})
                list(APPEND file_deps $<$<CONFIG:${cfg}>:${file}>)
            endforeach()
        endforeach()
        if(NOT CMAKE_CONFIGURATION_TYPES AND CMAKE_BUILD_TYPE)
            list(APPEND file_deps ${FILES_${CMAKE_BUILD_TYPE}})
        endif()

    endforeach(dest_idx)

    add_custom_target(
        ${target_name} ALL ${commands}
        DEPENDS ${file_deps}
        VERBATIM
    )

endfunction()

# Has to sit outside of a function, as otherwise CMAKE_CURRENT_LIST_DIR will
# evaluate to the directory of the including CMakeLists.txt
set(COPY_FILES_SCRIPT_TEMPLATE "${CMAKE_CURRENT_LIST_DIR}/copy_files.cmake.in")
