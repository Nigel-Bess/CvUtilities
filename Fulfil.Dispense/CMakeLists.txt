# Setup the project

# Specify a minimum version of CMake
cmake_minimum_required(VERSION 3.22)
cmake_policy(VERSION 3.22)

# Project name and settings
project(Fulfil.Dispense
        VERSION 3.1
        DESCRIPTION "Higher level API that allows clients to interface with depth cams via socket connection. Handles Drop camera nad Tray Camera Queries."
        LANGUAGES CXX)


if (CMAKE_VERSION VERSION_LESS "3.1")
    add_definitions(-std=c++17)
else()
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
find_package(Threads REQUIRED)

include(GNUInstallDirs)
message("Using C compiler ${CMAKE_C_COMPILER} and C++ compiler ${CMAKE_CXX_COMPILER}")

cmake_path(GET CMAKE_SOURCE_DIR PARENT_PATH COMPUTER_VISION_MODULES_PARENT_PATH)
message("Found project root at ${COMPUTER_VISION_MODULES_PARENT_PATH}")

option(BUILD_TESTS "Enable CTesting for dispense code." OFF)

IF(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTS)
    IF(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/googletest-src)
        #Don't install google test
        # Add googletest directly to our build. This defines
        # the gtest and gtest_main targets.
        add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                EXCLUDE_FROM_ALL)
    else()
        # ------ Setup Google Test
        configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
        execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
                RESULT_VARIABLE result
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download)
        if(result)
            message(FATAL_ERROR "CMake step for googletest failed: ${result}")
        endif()
        execute_process(COMMAND ${CMAKE_COMMAND} --build .
                RESULT_VARIABLE result
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download)
        if(result)
            message(FATAL_ERROR "Build step for googletest failed: ${result}")
        endif()
        # Prevent overriding the parent project's compiler/linker
        # settings on Windows
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

        # Add googletest directly to our build. This defines
        # the gtest and gtest_main targets.
        add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                EXCLUDE_FROM_ALL)

        # The gtest/gtest_main targets carry header search path
        # dependencies automatically when using CMake 2.8.11 or
        # later. Otherwise we have to add them here ourselves.
    endif()
    include(CTest)
    enable_testing()
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

add_subdirectory(src)
add_subdirectory(app)

