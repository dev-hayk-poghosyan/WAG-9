add_library(kainjow-mustache INTERFACE IMPORTED GLOBAL)

set_target_properties(
    kainjow-mustache PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${KAINJOW_MUSTACHE_INCLUDE_DIR}"
)
