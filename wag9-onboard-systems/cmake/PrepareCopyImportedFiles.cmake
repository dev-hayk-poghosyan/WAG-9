#
# prepare_copy_imported_files(<out_var> <imported_targets>)
#
# Sets <out_var> to a list of directives to copy the files associated with each
# one of <imported_targets>. The generated directives are meant to be passed to
# generate_copy_files_target(), following the "DESTINATION <dir>" arguments.
#
# The list of files to be copied is composed by querying the LOCATION_<CONFIG>
# properties of each one of <imported_targets>. We don't attempt to recurse
# into targets listed in INTERFACE_LINK_LIBRARIES property. Should such a
# functionality be required, it should be implemented in a separate function,
# with its output being fed as input into this one.
#
function(prepare_copy_imported_files out_var)

    set(directives "")

    foreach(target ${ARGN})

        if(CMAKE_CONFIGURATION_TYPES)
            # Multi-configuration setup, such as Visual Studio.
            foreach(cfg ${CMAKE_CONFIGURATION_TYPES})
                configuration_specific_target_processing(${cfg} ${target} directives)
            endforeach(cfg)
        else()
            # Single-configuration setup, such as NMake or Ninja.
            configuration_specific_target_processing("${CMAKE_BUILD_TYPE}" ${target} directives)
        endif()

    endforeach(target)

    set(${out_var} ${directives} PARENT_SCOPE)

endfunction()


function(configuration_specific_target_processing cfg target append_var)

    set(directives ${${append_var}})
    string(TOUPPER ${cfg} ucfg)

    get_target_property(location ${target} "LOCATION_${ucfg}")
    if(location)
        list(APPEND directives CONFIGURATIONS ${cfg} FILES "${location}")
    endif()

    set(${append_var} ${directives} PARENT_SCOPE)

endfunction()
