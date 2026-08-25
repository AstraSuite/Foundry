cmake_minimum_required(VERSION 3.19)

execute_process(
    COMMAND git describe --tags --abbrev=0
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_TAG_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

if(GIT_TAG_VERSION MATCHES "^v?([0-9]+\\.[0-9]+.*)")
    string(REGEX REPLACE "^v" "" ASTRA_VERSION_RAW "${GIT_TAG_VERSION}")
    set(ASTRA_VERSION_STR "${ASTRA_VERSION_RAW}")
else()
    set(ASTRA_VERSION_STR "1.3.0")
endif()

project(astra-foundry VERSION ${ASTRA_VERSION_STR} LANGUAGES CXX)

add_compile_definitions(ASTRA_VERSION="${ASTRA_VERSION_STR}")

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

