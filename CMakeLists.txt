cmake_minimum_required(VERSION 3.19)

# Version selection: pass -DASTRA_FOUNDRY_VERSION=X.Y.Z to pin the version manually,
# otherwise derive it from the most recent git tag (e.g. v1.2.3).
set(ASTRA_FOUNDRY_VERSION ""
    CACHE STRING "Version of astra-foundry (default: latest git tag)")

if(NOT ASTRA_FOUNDRY_VERSION)
    find_package(Git QUIET)
    if(GIT_EXECUTABLE)
        execute_process(
            COMMAND "${GIT_EXECUTABLE}" describe --tags --match "v[0-9]*" --abbrev=0 HEAD
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            RESULT_VARIABLE git_describe_result
            OUTPUT_VARIABLE git_describe_version
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        if(git_describe_result EQUAL 0 AND git_describe_version)
            string(REGEX REPLACE "^v" "" ASTRA_FOUNDRY_VERSION "${git_describe_version}")
        endif()
    endif()
endif()

if(NOT ASTRA_FOUNDRY_VERSION)
    # Dev version: YYYY.MM.DD.git.HASH
    find_package(Git REQUIRED)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" log -1 --format=%cd --date=format:%Y.%m.%d
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE git_date
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" rev-parse --short HEAD
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE git_hash
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(ASTRA_FOUNDRY_VERSION "${git_date}.git.${git_hash}")
endif()

project(astra-foundry VERSION ${ASTRA_FOUNDRY_VERSION} LANGUAGES CXX)

add_compile_definitions(ASTRA_VERSION="${ASTRA_FOUNDRY_VERSION}")

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT OR CMAKE_INSTALL_PREFIX STREQUAL "/usr/local")
    set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Default install prefix" FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Quick QuickControls2 ShaderTools Concurrent Network DBus Svg)

qt_standard_project_setup(REQUIRES 6.5)

# C++ Source files
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    "src/*.cpp"
    "src/*.hpp"
)

qt_add_executable(foundry ${PROJECT_SOURCES})

qt_add_shaders(foundry "blob_shaders"
    BATCHABLE
    OPTIMIZED
    PREFIX "/"
    GLSL "300es,120,150,330,450"
    HLSL 50
    MSL 20000
    FILES
        shaders/blob.vert
        shaders/blob.frag
)

target_include_directories(foundry PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/src/marketplace
    ${CMAKE_CURRENT_SOURCE_DIR}/src/marketplace/plugins
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tray
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cli
    ${CMAKE_CURRENT_SOURCE_DIR}/src/blobs
)

target_link_libraries(foundry PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::ShaderTools
    Qt6::Concurrent
    Qt6::Network
    Qt6::DBus
    Qt6::Svg
)

# Embed qml and assets resources
qt_add_resources(foundry "resources"
    PREFIX "/"
    FILES
        assets/icons/astra-foundry.svg
        assets/icons/tray-dark.svg
        assets/icons/tray-light.svg
        assets/fonts/GoogleSansFlex-VariableFont_GRAD,ROND,opsz,slnt,wdth,wght.ttf
)

# Add QML files with CONFIGURE_DEPENDS
file(GLOB_RECURSE QML_FILES CONFIGURE_DEPENDS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "qml/*.qml" "qml/**/qmldir")
qt_add_resources(foundry "qml_resources"
    PREFIX "/"
    FILES ${QML_FILES}
)

find_package(Qt6 QUIET COMPONENTS LinguistTools)

if(Qt6LinguistTools_FOUND)
    qt_add_translations(foundry
        TS_FILES translations/foundry_de.ts
        RESOURCE_PREFIX "/i18n"
    )
else()
    message(STATUS "Qt Linguist tools not found, Foundry is built without translations")
endif()

option(ASTRA_BUILD_TESTS "Build the Foundry unit tests" OFF)

if(ASTRA_BUILD_TESTS)
    enable_testing()
    find_package(Qt6 REQUIRED COMPONENTS Test)

    qt_add_executable(tst_parsers
        tests/tst_parsers.cpp
        src/marketplace/pluginprocess.cpp
        src/marketplace/pluginprocess.hpp
        src/marketplace/plugins/packageplugin.hpp
        src/marketplace/plugins/pacmanplugin.cpp
        src/marketplace/plugins/pacmanplugin.hpp
        src/marketplace/plugins/flatpakplugin.cpp
        src/marketplace/plugins/flatpakplugin.hpp
    )

    target_include_directories(tst_parsers PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/marketplace
        ${CMAKE_CURRENT_SOURCE_DIR}/src/marketplace/plugins
    )

    target_link_libraries(tst_parsers PRIVATE
        Qt6::Core
        Qt6::Network
        Qt6::Test
    )

    add_test(NAME parsers COMMAND tst_parsers)
endif()

# Installation
include(GNUInstallDirs)

install(TARGETS foundry
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES assets/foundry.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)

install(FILES assets/icons/astra-foundry.svg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
    RENAME astra-foundry.svg
)

install(FILES assets/icons/tray-dark.svg assets/icons/tray-light.svg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
)

install(FILES assets/polkit/com.astra-foundry.pacman.policy
    DESTINATION ${CMAKE_INSTALL_DATADIR}/polkit-1/actions
)

install(FILES assets/completions/foundry.bash
    DESTINATION ${CMAKE_INSTALL_DATADIR}/bash-completion/completions
    RENAME foundry
)

install(FILES assets/completions/_foundry
    DESTINATION ${CMAKE_INSTALL_DATADIR}/zsh/site-functions
)

install(FILES assets/completions/foundry.fish
    DESTINATION ${CMAKE_INSTALL_DATADIR}/fish/vendor_completions.d
)

