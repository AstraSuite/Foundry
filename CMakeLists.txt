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
    set(ASTRA_VERSION_STR "1.1.0")
endif()

project(AstraMarket VERSION ${ASTRA_VERSION_STR} LANGUAGES CXX)

add_compile_definitions(ASTRA_VERSION="${ASTRA_VERSION_STR}")

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT OR CMAKE_INSTALL_PREFIX STREQUAL "/usr/local")
    set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Default install prefix" FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Quick QuickControls2 ShaderTools Concurrent Network DBus Svg)

qt_standard_project_setup(REQUIRES 6.5)

# Custom qsb command for blob shaders
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/shaders/blob.vert.qsb
           ${CMAKE_CURRENT_SOURCE_DIR}/shaders/blob.frag.qsb
    COMMAND /usr/lib/qt6/bin/qsb --glsl 300es,120,150,330,450 -b -O --hlsl 50 --msl 20000 -o ${CMAKE_CURRENT_SOURCE_DIR}/shaders/blob.vert.qsb ${CMAKE_CURRENT_SOURCE_DIR}/shaders/blob.vert
    COMMAND /usr/lib/qt6/bin/qsb --glsl 300es,120,150,330,450 -b -O --hlsl 50 --msl 20000 -o ${CMAKE_CURRENT_SOURCE_DIR}/shaders/blob.frag.qsb ${CMAKE_CURRENT_SOURCE_DIR}/shaders/blob.frag
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/blob.vert
            ${CMAKE_CURRENT_SOURCE_DIR}/shaders/blob.frag
    COMMENT "Compiling QSB shaders"
)

# C++ Source files
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    "src/*.cpp"
    "src/*.hpp"
)

list(APPEND PROJECT_SOURCES
    shaders/blob.vert.qsb
    shaders/blob.frag.qsb
)

qt_add_executable(astra ${PROJECT_SOURCES})

target_include_directories(astra PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/src/marketplace
    ${CMAKE_CURRENT_SOURCE_DIR}/src/marketplace/plugins
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tray
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cli
    ${CMAKE_CURRENT_SOURCE_DIR}/src/blobs
)

target_link_libraries(astra PRIVATE
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
qt_add_resources(astra "resources"
    PREFIX "/"
    FILES
        assets/icons/AstraMarket.svg
        assets/icons/tray-dark.svg
        assets/icons/tray-light.svg
        assets/fonts/GoogleSansFlex-VariableFont_GRAD,ROND,opsz,slnt,wdth,wght.ttf
        shaders/blob.vert.qsb
        shaders/blob.frag.qsb
)

# Add QML files with CONFIGURE_DEPENDS
file(GLOB_RECURSE QML_FILES CONFIGURE_DEPENDS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "qml/*.qml" "qml/**/qmldir")
qt_add_resources(astra "qml_resources"
    PREFIX "/"
    FILES ${QML_FILES}
)

# Installation
include(GNUInstallDirs)

install(TARGETS astra
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES assets/astra.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)

install(FILES assets/icons/AstraMarket.svg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
    RENAME AstraMarket.svg
)

install(FILES assets/icons/tray-dark.svg assets/icons/tray-light.svg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
)

install(FILES assets/polkit/com.astramarket.pacman.policy
    DESTINATION ${CMAKE_INSTALL_DATADIR}/polkit-1/actions
)

