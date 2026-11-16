add_library(csv-parser INTERFACE IMPORTED GLOBAL)

set_target_properties(
    csv-parser PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CSVPARSER_INCLUDE_DIR}"
)
