# Usage:
# compile_proto_files(
#     proto_files generated_sources ${dir_with_proto_files}
#     ${dir_for_generated_sources} ${additional_include_dir} ${file_names_without_proto_extension})
#
function(compile_proto_files out_proto_files out_generated_sources
         proto_files_dir generated_sources_dir ext_include_dir)

    set(proto_files "")
    set(generated_sources "")
    set(commands "")
    set(error_format "gcc")

    if (MSVC)
        set(error_format "msvs")
    endif()

    set(generated_sources_tmp_dir "${generated_sources_dir}/.proto-temp")

    list(APPEND commands COMMAND ${CMAKE_COMMAND} -E make_directory ${generated_sources_tmp_dir})

    foreach(base_name ${ARGN})
        set(proto_file "${proto_files_dir}/${base_name}.proto")
        set(cpp_file_name "${base_name}.pb.cc")
        set(h_file_name "${base_name}.pb.h")

        list(APPEND proto_files ${proto_file})
        list(APPEND generated_sources "${generated_sources_dir}/${cpp_file_name}")
        list(APPEND generated_sources "${generated_sources_dir}/${h_file_name}")
        set(includes "")
        foreach(ext_include ${ext_include_dir})
           list(APPEND includes -I ${ext_include})
        endforeach()
        list(APPEND commands COMMAND protobuf::protoc -I ${proto_files_dir} ${includes}
            --error_format=${error_format} --cpp_out ${generated_sources_tmp_dir} ${proto_file})

        # This makes sure we are not updating the files that haven't changed.
        list(APPEND commands COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${generated_sources_tmp_dir}/${cpp_file_name}"
            "${generated_sources_tmp_dir}/${h_file_name}" ${generated_sources_dir})
    endforeach()

    add_custom_command(
        OUTPUT ${generated_sources}
        DEPENDS ${proto_files}
        ${commands}
    )

    set(${out_proto_files} ${proto_files} PARENT_SCOPE)
    set(${out_generated_sources} ${generated_sources} PARENT_SCOPE)

endfunction()
