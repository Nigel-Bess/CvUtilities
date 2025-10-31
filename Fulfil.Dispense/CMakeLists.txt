# Setup the project

# Specify a minimum version of CMake
cmake_minimum_required(VERSION 3.22)
cmake_policy(VERSION 3.22)

# Project name and settings
project(Fulfil.Dispense
        VERSION 3.1
        DESCRIPTION "Higher level API that allows clients to interface with depth cams via socket connection. Handles Drop camera nad Tray Camera Queries."
        LANGUAGES CXX)

set(OrbbecSDK_DIR "${PROJECT_SOURCE_DIR}/../Fulfil.OrbbecUtils/lib/orbbecsdk/lib")
find_package(OrbbecSDK REQUIRED)

if (CMAKE_VERSION VERSION_LESS "3.1")
    add_definitions(-std=c++17)
else()
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
find_package(Threads REQUIRED)

# Set up Ninja build system
if(NOT CMAKE_GENERATOR MATCHES "Ninja")
    message(FATAL_ERROR "Please run cmake with -GNinja option.")
endif()

include(GNUInstallDirs)
message("Using C compiler ${CMAKE_C_COMPILER} and C++ compiler ${CMAKE_CXX_COMPILER}")

cmake_path(GET CMAKE_SOURCE_DIR PARENT_PATH COMPUTER_VISION_MODULES_PARENT_PATH)
message("Found project root at ${COMPUTER_VISION_MODULES_PARENT_PATH}")

option(BUILD_TESTS "Enable CTesting for dispense code." OFF)

IF(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTS)
    include(CTest)
    enable_testing()

    include(FetchContent)

    # Self-contained GTest (does not install anything)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_GTEST ON CACHE BOOL "" FORCE)
    set(BUILD_GMOCK ON CACHE BOOL "" FORCE)

    # Fetch googletest at configure time
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )

    # This will download (if needed) and add_subdirectory() googletest
    FetchContent_MakeAvailable(googletest)

    # Add tests
    add_subdirectory(test)
endif()



if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    #This is where you should call add_subdirectory for Fulfil libraries.
    option(RESET_FULFIL_DEFAULTS "Returns Fulfil Cmake flag for project back to original state." OFF)
    if (RESET_FULFIL_DEFAULTS)
        unset(FULFIL_LOG_DIR CACHE)
        unset(FULFIL_INI_DIR CACHE)
        unset(FULFIL_DISABLE_JSON CACHE)
    endif()
    unset(RESET_FULFIL_DEFAULTS CACHE)
endif()
string(
        COMPARE EQUAL
        "${CMAKE_SOURCE_DIR}"
        "${PROJECT_SOURCE_DIR}"
        PROJECT_IS_TOP_LEVEL)


# Set up third party common dependencies
IF (NOT ${Protobuf_FOUND})
    set(Protobuf_USE_STATIC_LIBS ON)
    find_package(Protobuf REQUIRED)
    message(STATUS "Using protobuf ${Protobuf_VERSION}")
ENDIF()
IF (NOT ${gRPC_FOUND})
    find_package(gRPC CONFIG REQUIRED)
    message(STATUS "Using gRPC ${gRPC_VERSION}")
ENDIF()

#set(TARGET_JSON_TOOL "json")

add_subdirectory(${COMPUTER_VISION_MODULES_PARENT_PATH}/third-party ${CMAKE_CURRENT_BINARY_DIR}/libs/third-party)
add_subdirectory(${COMPUTER_VISION_MODULES_PARENT_PATH}/Fulfil.DepthCam ${CMAKE_CURRENT_BINARY_DIR}/libs/Fulfil.DepthCam)

# Build src and app in their own build subfolders
set(BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build)
message("Building into directory: ${BINARY_DIR}")

add_subdirectory(src)
add_subdirectory(app)

# Set binary output directories for all targets
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY "${BINARY_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${BINARY_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${BINARY_DIR}")

