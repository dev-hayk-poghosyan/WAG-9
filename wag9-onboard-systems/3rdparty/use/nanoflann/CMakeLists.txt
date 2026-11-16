add_library(nanoflann INTERFACE IMPORTED GLOBAL)

set_target_properties(
    nanoflann PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${NANOFLANN_INCLUDE_DIR}"
)
