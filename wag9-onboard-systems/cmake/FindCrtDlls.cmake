#
# Finds the CRT DLLs that need to be copied in the same directory as executables.
#
function(find_crt_dlls out_var_name)

    set(${out_var_name} "" PARENT_SCOPE)

    if(MSVC)
        if(NOT CMAKE_VS_PLATFORM_NAME)
            # CMAKE_VS_PLATFORM_NAME is only set when using the Visual Studio generator.
            # It's not set when using nmake or Ninja with Microsoft's compilers, so we guess it.
            if(CMAKE_SIZEOF_VOID_P EQUAL 8)
                set(CMAKE_VS_PLATFORM_NAME x64)
            else()
                set(CMAKE_VS_PLATFORM_NAME x86)
            endif()
        endif()
    
        get_filename_component(linker_dir "${CMAKE_LINKER}" PATH)
        string(REGEX REPLACE "/VC/.*" "/VC" vc_dir ${linker_dir})

        # When MSVC_TOOLSET_VERSION is "142", we want toolset_dot_zero to be "14.0"
        string(REGEX REPLACE "([0-9]+)([0-9])" "\\1.0" toolset_dot_zero ${MSVC_TOOLSET_VERSION})

        set(redist_version "<no-version>")
        set(redist_version_file "${vc_dir}/Auxiliary/Build/Microsoft.VCRedistVersion.default.txt")
        if (EXISTS "${redist_version_file}")
            file(STRINGS "${redist_version_file}" redist_version)
        endif()

        set(crt_dir_name "Microsoft.VC${MSVC_TOOLSET_VERSION}.CRT")

        find_path(
            VC_REDIST_DIR ${crt_dir_name}
            HINTS "${vc_dir}/Redist/MSVC/${redist_version}/${CMAKE_VS_PLATFORM_NAME}"
            DOC "The directory containing the \"${crt_dir_name}\" subdirectory"
            NO_CMAKE_ENVIRONMENT_PATH
            NO_SYSTEM_ENVIRONMENT_PATH
        )
        if(NOT VC_REDIST_DIR)
            message(
                FATAL_ERROR
                "Could not find Visual Studio redistributables. "
                "They are typically located in a directory like "
                "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\VC\\Redist\\MSVC\\14.24.28127\\${CMAKE_VS_PLATFORM_NAME} "
                "Please set the VC_REDIST_DIR accordingly."
            )
        elseif(NOT EXISTS "${VC_REDIST_DIR}/${crt_dir_name}")
            message(
                FATAL_ERROR
                "Failed to find the \"${crt_dir_name}\" subdirectory under VC_REDIST_DIR"
            )
        endif()

        file(GLOB crt_dlls "${VC_REDIST_DIR}/${crt_dir_name}/*.dll")

        set(${out_var_name} ${crt_dlls} PARENT_SCOPE)

    endif() # if(MSVC)

endfunction()
