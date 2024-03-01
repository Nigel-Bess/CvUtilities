# Setup the project

# Specify a minimum version of CMake
cmake_minimum_required(VERSION 3.20)
# set(GRPC_FETCHCONTENT ON)
include(common.cmake)

# Project name and settings
project(
        Fulfil.Utils
        VERSION 2.0
        DESCRIPTION "A library of generic utils for c++ projects"
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
if (CMAKE_VERSION VERSION_LESS "3.1")
   add_definitions(-std=c++17)
else()
   set(CMAKE_CXX_STANDARD 17)
   set(CMAKE_CXX_STANDARD_REQUIRED ON)
   set(CMAKE_CXX_EXTENSIONS OFF)
endif()


IF(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
   IF(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/googletest-src)
      #Don't install google test
      message("-- Found directory googletest-src, will not install googletest")
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
      include(CTest)
      enable_testing()

      # The gtest/gtest_main targets carry header search path
      # dependencies automatically when using CMake 2.8.11 or
      # later. Otherwise we have to add them here ourselves.
      if (CMAKE_VERSION VERSION_LESS 2.8.11)
         include_directories("${gtest_SOURCE_DIR}/include")
      endif()
   endif()
   add_subdirectory(test)

endif()

#set(FULFIL_LOG_DIR "/home/fulfil/code/Fulfil.Dispense/logs")
#set(FULFIL_INI_DIR "/home/fulfil/code/Fulfil.Dispense/configs")

# Allow for Macro definition on cmake command line. Default is build root on current system.
if (DEFINED FULFIL_LOG_DIR)
   message("-- Using custom defined logger directory ${FULFIL_LOG_DIR}")
else()
   set(FULFIL_LOG_DIR "${CMAKE_SOURCE_DIR}/logs")
   message("-- Using default logger directory ${FULFIL_LOG_DIR}")
endif()

if (DEFINED FULFIL_INI_DIR)
   message("-- Using custom defined config directory ${FULFIL_INI_DIR}")
else ()
   set(FULFIL_INI_DIR "${CMAKE_SOURCE_DIR}/configs")
   message("-- Using default config directory ${FULFIL_INI_DIR}")
endif()

add_subdirectory(app)
add_subdirectory(src)
