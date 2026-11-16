set(
    ts_files
    app_en.ts
    app_ru.ts
    app_lv.ts
)

# Unfortunately we can't use qt5_create_translation() because of QTBUG-41736
add_custom_target(
    RefreshTranslations ALL SOURCES ${ts_files}
    COMMAND ${Qt5_LUPDATE_EXECUTABLE} ../src ../ui
    -locations none -no-ui-lines -ts ${ts_files}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    VERBATIM
)

# Note that we use "-locations none" and "-no-ui-lines" in order to avoid git conflicts.
# Before running Qt Linguist it's recommended to run the RefreshTranslationsWithLines
# manually. However, don't commit that version to git! Rebuild the solution again to
# regenerate *.ts files without line numbers.

add_custom_target(
    RefreshTranslationsWithLines SOURCES ${ts_files}
    COMMAND ${Qt5_LUPDATE_EXECUTABLE} ../src ../ui
    -locations absolute -ts ${ts_files}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    VERBATIM
)

qt5_add_translation(qm_files ${ts_files})
add_custom_target(CompileTranslations ALL DEPENDS ${qm_files})

add_dependencies(CompileTranslations RefreshTranslations)

set(QM_FILES ${qm_files} PARENT_SCOPE)
