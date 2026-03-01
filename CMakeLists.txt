cmake_minimum_required(VERSION 3.20)
project(GeopoliticalSimulator VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Opções de build
option(GPS_BUILD_TESTS "Build unit tests" ON)
option(GPS_ENABLE_MODDING "Enable modding support" ON)
option(GPS_DEBUG_SIMULATION "Enable simulation debug output" OFF)

# ===== FTXUI (Terminal UI Library) =====
include(FetchContent)
FetchContent_Declare(
    ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
    GIT_TAG v5.0.0
)
FetchContent_GetProperties(ftxui)
if(NOT ftxui_POPULATED)
    FetchContent_Populate(ftxui)
    set(FTXUI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(FTXUI_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(FTXUI_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    add_subdirectory(${ftxui_SOURCE_DIR} ${ftxui_BINARY_DIR})
endif()

# Diretórios de include
include_directories(${CMAKE_SOURCE_DIR}/include)

# Coletar fontes
file(GLOB_RECURSE CORE_SOURCES "src/core/*.cpp")
file(GLOB_RECURSE SYSTEMS_SOURCES "src/systems/*.cpp")
file(GLOB_RECURSE WORLD_SOURCES "src/world/*.cpp")
file(GLOB_RECURSE UI_SOURCES "src/ui/*.cpp")
file(GLOB_RECURSE UTILS_SOURCES "src/utils/*.cpp")
file(GLOB_RECURSE DATA_SOURCES "src/data/*.cpp")

# Biblioteca principal do simulador
add_library(gps_engine STATIC
    ${CORE_SOURCES}
    ${SYSTEMS_SOURCES}
    ${WORLD_SOURCES}
    ${UI_SOURCES}
    ${UTILS_SOURCES}
    ${DATA_SOURCES}
)

target_include_directories(gps_engine PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(gps_engine PUBLIC
    ftxui::screen
    ftxui::dom
    ftxui::component
)

target_compile_definitions(gps_engine PUBLIC
    $<$<BOOL:${GPS_ENABLE_MODDING}>:GPS_MODDING_ENABLED>
    $<$<BOOL:${GPS_DEBUG_SIMULATION}>:GPS_DEBUG_SIM>
)

# Executável principal
add_executable(gps_game src/main.cpp)
target_link_libraries(gps_game PRIVATE
    gps_engine
)

# Copiar dados para build
add_custom_command(TARGET gps_game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data
    $<TARGET_FILE_DIR:gps_game>/data
)

# Tests
if(GPS_BUILD_TESTS)
    enable_testing()
    file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")
    if(TEST_SOURCES)
        add_executable(gps_tests ${TEST_SOURCES})
        target_link_libraries(gps_tests PRIVATE gps_engine)
        add_test(NAME GPSTests COMMAND gps_tests)
    endif()
endif()

message(STATUS "=== Geopolitical Simulator ===")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Modding: ${GPS_ENABLE_MODDING}")
message(STATUS "Debug Sim: ${GPS_DEBUG_SIMULATION}")
