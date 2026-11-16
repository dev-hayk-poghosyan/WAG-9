set(CURRENT_LIST_DIR ${CMAKE_CURRENT_LIST_DIR})

set(pre_configure_file  ${CMAKE_CURRENT_LIST_DIR}/GitVersion.h.in)
set(post_configure_file ${CMAKE_CURRENT_BINARY_DIR}/GitVersion.h)

function(RetriveGitHeadHash)
    # Get the latest abbreviated commit hash of the working branch
    execute_process(
        COMMAND git log -1 --format=%h
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        OUTPUT_VARIABLE GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    configure_file(${pre_configure_file} ${post_configure_file} @ONLY)

endfunction()
