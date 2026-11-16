cmake_minimum_required(VERSION 3.11.1)

project("WAG9")

if(POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

if(POLICY CMP0143)
  cmake_policy(SET CMP0143 NEW)
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "This project requires a 64-bit configuration.")
endif()

# In multi-configuration IDEs, only allow Debug and RelWithDebInfo
if(CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES Debug RelWithDebInfo)
    set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}"
        CACHE STRING "Build Configurations" FORCE)
endif()

include(cmake/CreateCopyFilesTarget.cmake)
include(cmake/PrepareCopyImportedFiles.cmake)
include(cmake/FindCrtDlls.cmake)

if (NOT DEPS_BUILD_DIR)
    set(DEPS_BUILD_DIR "" CACHE PATH "Top-level build directory for dependencies")
    message(FATAL_ERROR "\
Please build 3rd-party dependencies first and then set the DEPS_BUILD_DIR \
variable appropriately. See README.md for more details.\
")
endif()

include("${DEPS_BUILD_DIR}/railroad-control-deps.cmake")

set (CMAKE_CXX_STANDARD 17)

include_directories(.)

if(CMAKE_CXX_COMPILER_ID STREQUAL GNU)
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-function")
elseif(MSVC)
    # -wd4146: Don't warn on negating an unsigned type.
    # -wd4244: Don't warn on narrowing conversions.
    # -wd4267: Don't warn on narrowing conversions from size_t.
    # -wd4305: Don't warn on floating point narrowing conversions.
    # -wd4800: Don't warn on implicit conversions from int to bool.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4146 -wd4244 -wd4267 -wd4305 -wd4800 -utf-8")
endif()

if(MSVC)
    # Some of those warnings are unavoidable as they are coming from 3rd-party libraries.
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)

    # Use the AVX instruction set.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch:AVX")

    if(CMAKE_GENERATOR MATCHES "Visual Studio")
        # Enable parallel compilation. Note that we don't want to do that for other
        # generators such as Ninja, as those provide their own parallelization.
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -MP")
    endif()
endif()

if(WIN32)
    if(NOT MSVC14)
        message(WARNING "Only Visual Studio 2015 is officially supported")
    endif()

    # Prevent min() and max() macros being defined by windows.h
    add_definitions(-DNOMINMAX)

    # Include less stuff into windonws.h
    add_definitions(-DWIN32_LEAN_AND_MEAN)

    # The minimum version of Windows we support.
    add_definitions(-D_WIN32_WINNT=0x0600) # Windows Vista
endif()

# http://doc.qt.io/qt-5/qmessagelogcontext.html
# By default QMessageLogContext only works in Debug mode.
# To make it permanent for all modes, we need to define QT_MESSAGELOGCONTEXT
add_definitions("-DQT_MESSAGELOGCONTEXT")

# add_subdirectory() would be preferable to include() here, as it hides
# variables from the parent scope. Unfortunately, some of our dependencies
# generate non-GLOBAL IMPORTED targets, making them hidden from the parent
# scope as well. For those, we use include().
#
include("3rdparty/use/qt5/CMakeLists.txt")
include("3rdparty/use/boost/CMakeLists.txt")
include("3rdparty/use/zmq/CMakeLists.txt")
include("3rdparty/use/cppzmq/CMakeLists.txt")
include("3rdparty/use/zlib/CMakeLists.txt")
include("3rdparty/use/protobuf/CMakeLists.txt")
include("3rdparty/use/pugixml/CMakeLists.txt")
include("3rdparty/use/bugtrap/CMakeLists.txt")
add_subdirectory("3rdparty/use/glm")
add_subdirectory("3rdparty/use/nanoflann")
add_subdirectory("3rdparty/use/csv-parser")
add_subdirectory("3rdparty/use/kainjow-mustache")

# Sub-projects
add_subdirectory(src)
add_subdirectory(translations)


if(WIN32)
    # Copy some DLLs to the staging dir.

    find_crt_dlls(crt_dlls)

    # Imported DLLs.
    set(
        imported_targets
        Qt5::Core Qt5::Xml Qt5::Gui Qt5::Widgets Qt5::Sql Qt5::Charts
        Qt5::Network Qt5::Multimedia Qt5::Svg
    )
    prepare_copy_imported_files(COPY_IMPORTED_FILE_DIRECTIVES ${imported_targets})

    get_filename_component(QT5_PLUGINS_DIR "${Qt5_DIR}/../../../plugins" REALPATH)

    file(GLOB all_qt_qm_files "${Qt5_DIR}/../../../translations/*.qm")
    set(import_qt_locales en ru)

    # We want Qt's *.qm files in order to have translations for things like
    # standard dialog buttons.
    set(QT5_QM_FILES "")

    foreach(qm_file ${all_qt_qm_files})
        # Note that qt_${lang}.qm depends on the other ones we list here.
        # Omitting even one of them will cause qt_${lang}.qm to fail loading.
        if(qm_file MATCHES "/(qt_|qtbase_|qtscript_|qtquick1_|qtmultimedia_|qtxmlpatterns_)([^/]*).qm$")
            if(CMAKE_MATCH_2 IN_LIST import_qt_locales)
                list(APPEND QT5_QM_FILES ${qm_file})
            endif()
        endif()
    endforeach()

    # A dummy library to avoid a circular dependency.
    add_library(dummy STATIC CMakeLists.txt)

    # Generate the target that will actually do the copying.
    create_copy_files_target(
        CopyDependencies

        DESTINATION "$<TARGET_FILE_DIR:dummy>"
            ${COPY_IMPORTED_FILE_DIRECTIVES}

            CONFIGURATIONS ALL FILES ${crt_dlls}

            CONFIGURATIONS Release MinSizeRel RelWithDebInfo FILES
                "${ZLIB_ROOT}/bin/zlib.dll"
            CONFIGURATIONS Debug FILES
                "${ZLIB_ROOT}/bin/zlibd.dll"

            CONFIGURATIONS ALL FILES
                "${BugTrap_DIR}/dbghelp.dll"
                "${CMAKE_SOURCE_DIR}/data/Wag9.xml"
            CONFIGURATIONS Release MinSizeRel RelWithDebInfo FILES
                "${BugTrap_DIR}/BugTrapU-x64.dll"
            CONFIGURATIONS Debug FILES
                "${BugTrap_DIR}/BugTrapUD-x64.dll"

        DESTINATION "$<TARGET_FILE_DIR:dummy>/platforms"
            CONFIGURATIONS Release MinSizeRel RelWithDebInfo FILES
                "${QT5_PLUGINS_DIR}/platforms/qwindows.dll"
            CONFIGURATIONS Debug FILES
                "${QT5_PLUGINS_DIR}/platforms/qwindowsd.dll"

        DESTINATION "$<TARGET_FILE_DIR:dummy>/sqldrivers"
            CONFIGURATIONS Release MinSizeRel RelWithDebInfo FILES
                "${QT5_PLUGINS_DIR}/sqldrivers/qsqlite.dll"
            CONFIGURATIONS Debug FILES
                "${QT5_PLUGINS_DIR}/sqldrivers/qsqlited.dll"

        DESTINATION "$<TARGET_FILE_DIR:dummy>/styles"
            CONFIGURATIONS Release MinSizeRel RelWithDebInfo FILES
                "${QT5_PLUGINS_DIR}/styles/qwindowsvistastyle.dll"
            CONFIGURATIONS Debug FILES
                "${QT5_PLUGINS_DIR}/styles/qwindowsvistastyled.dll"

        DESTINATION "$<TARGET_FILE_DIR:dummy>/imageformats"
            CONFIGURATIONS Release MinSizeRel RelWithDebInfo FILES
                "${QT5_PLUGINS_DIR}/imageformats/qgif.dll"
                "${QT5_PLUGINS_DIR}/imageformats/qjpeg.dll"
                "${QT5_PLUGINS_DIR}/imageformats/qtga.dll"
                "${QT5_PLUGINS_DIR}/imageformats/qtiff.dll"
                "${QT5_PLUGINS_DIR}/imageformats/qsvg.dll"
            CONFIGURATIONS Debug FILES
                "${QT5_PLUGINS_DIR}/imageformats/qgifd.dll"
                "${QT5_PLUGINS_DIR}/imageformats/qjpegd.dll"
                "${QT5_PLUGINS_DIR}/imageformats/qtgad.dll"
                "${QT5_PLUGINS_DIR}/imageformats/qtiffd.dll"
                "${QT5_PLUGINS_DIR}/imageformats/qsvgd.dll"

        DESTINATION "$<TARGET_FILE_DIR:dummy>/audio"
            CONFIGURATIONS Release MinSizeRel RelWithDebInfo FILES
                "${QT5_PLUGINS_DIR}/audio/qtaudio_windows.dll"
            CONFIGURATIONS Debug FILES
                "${QT5_PLUGINS_DIR}/audio/qtaudio_windowsd.dll"

        DESTINATION "$<TARGET_FILE_DIR:dummy>/mediaservice"
            CONFIGURATIONS Release MinSizeRel RelWithDebInfo FILES
                "${QT5_PLUGINS_DIR}/mediaservice/qtmedia_audioengine.dll"
            CONFIGURATIONS Debug FILES
                "${QT5_PLUGINS_DIR}/mediaservice/qtmedia_audioengined.dll"

        DESTINATION "$<TARGET_FILE_DIR:dummy>/translations"
            CONFIGURATIONS ALL FILES ${QM_FILES} ${QT5_QM_FILES}
        # Note that in single-configuration build systems (make, ninja)
        # We end up trying to copy ${QM_FILES} into themselves. CMake seems to handle
        # this situation gracefully.
    )

    set_target_properties(
        RefreshTranslations dummy CopyDependencies
        CompileTranslations RefreshTranslationsWithLines
        PROPERTIES FOLDER "Misc")

    set_target_properties(common neokon proto PROPERTIES FOLDER "Shared")

    set_target_properties(wag9_fs PROPERTIES FOLDER "WAG9")

    # CompileTranslations generates the files listed in ${QM_FILES}, which CopyDependencies copies.
    add_dependencies(CopyDependencies CompileTranslations)

    # Make each executable target depend on various Copy* tasks. This will ensure dependencies
    # will be copied even when we are building only a particular executable target, rather than
    # the whole project.

    add_dependencies(wag9_fs CopyDependencies)

    # For the Installer target.
    #add_subdirectory(packaging/windows)
endif()
