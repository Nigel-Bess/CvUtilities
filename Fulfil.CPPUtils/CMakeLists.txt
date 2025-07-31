# Specify a minimum version of CMake
cmake_minimum_required(VERSION 3.22)

# Setup the project

# Project name and settings
project(
        Fulfil.CPPUtils
        VERSION 1.0
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

set(INCLUDE_INSTALL_DIR /home/fulfil/code/Fulfil.ComputerVision/Fulfil.CPP/include/ )
set(LIB_INSTALL_DIR lib/ )
set(SYSCONFIG_INSTALL_DIR usr/bin/Fulfil.CPPUtils/ )
include(CMakePackageConfigHelpers)
configure_package_config_file(CMakeLists.txt.in
  ${CMAKE_CURRENT_BINARY_DIR}/Fulfil.CPPUtilsConfig.cmake
  INSTALL_DESTINATION ${LIB_INSTALL_DIR}/Fulfil.CPPUtils/cmake
  PATH_VARS INCLUDE_INSTALL_DIR SYSCONFIG_INSTALL_DIR)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/Fulfil.CPPUtilsConfigVersion.cmake
  VERSION 1.0.0
  COMPATIBILITY SameMajorVersion )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Fulfil.CPPUtilsConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/Fulfil.CPPUtilsConfigVersion.cmake
        DESTINATION ${LIB_INSTALL_DIR}/Fulfil.CPPUtils/cmake )

# set(GRPC_FETCHCONTENT ON)
if(MSVC)
  add_definitions(-D_WIN32_WINNT=0x600)
endif()

include(FetchContent)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
find_package(Threads REQUIRED)

find_package(OpenCV 4.6.0 REQUIRED)
message("-- Found OpenCV lib ${OpenCV_VERSION}!")
include_directories(${OpenCV_INCLUDE_DIRS})

if(GRPC_AS_SUBMODULE)
  # One way to build a projects that uses gRPC is to just include the
  # entire gRPC project tree via "add_subdirectory".
  # This approach is very simple to use, but the are some potential
  # disadvantages:
  # * it includes gRPC's CMakeLists.txt directly into your build script
  #   without and that can make gRPC's internal setting interfere with your
  #   own build.
  # * depending on what's installed on your system, the contents of submodules
  #   in gRPC's third_party/* might need to be available (and there might be
  #   additional prerequisites required to build them). Consider using
  #   the gRPC_*_PROVIDER options to fine-tune the expected behavior.
  #
  # A more robust approach to add dependency on gRPC is using
  # cmake's ExternalProject_Add (see cmake_externalproject/CMakeLists.txt).

  # Include the gRPC's cmake build (normally grpc source code would live
  # in a git submodule called "third_party/grpc", but this example lives in
  # the same repository as gRPC sources, so we just look a few directories up)
  add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/grpc EXCLUDE_FROM_ALL)
  message(STATUS "Using gRPC via add_subdirectory.")

  # After using add_subdirectory, we can now use the grpc targets directly from
  # this build.
  set(_PROTOBUF_LIBPROTOBUF libprotobuf)
  set(_REFLECTION grpc++_reflection)
  if(CMAKE_CROSSCOMPILING)
    find_program(_PROTOBUF_PROTOC protoc)
  else()
    set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
  endif()
  set(_GRPC_GRPCPP grpc++)
  if(CMAKE_CROSSCOMPILING)
    find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
  else()
    set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)
  endif()
elseif(GRPC_FETCHCONTENT)
  # Another way is to use CMake's FetchContent module to clone gRPC at
  # configure time. This makes gRPC's source code available to your project,
  # similar to a git submodule.
  message(STATUS "Using gRPC via add_subdirectory (FetchContent).")
  FetchContent_Declare(
    grpc
    GIT_REPOSITORY https://github.com/grpc/grpc.git
    # when using gRPC, you will actually set this to an existing tag, such as
    # v1.25.0, v1.26.0 etc..
    # For the purpose of testing, we override the tag used to the commit
    # that's currently under test.
    GIT_TAG        v1.54.0)
  FetchContent_MakeAvailable(grpc)

  # Since FetchContent uses add_subdirectory under the hood, we can use
  # the grpc targets directly from this build.
  set(_PROTOBUF_LIBPROTOBUF libprotobuf)
  set(_REFLECTION grpc++_reflection)
  set(_PROTOBUF_PROTOC $<TARGET_FILE:protoc>)
  set(_GRPC_GRPCPP grpc++)
  if(CMAKE_CROSSCOMPILING)
    find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
  else()
    set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)
  endif()
else()
  # This branch assumes that gRPC and all its dependencies are already installed
  # on this system, so they can be located by find_package().

  # Find Protobuf installation
  # Looks for protobuf-config.cmake file installed by Protobuf's cmake installation.
  option(protobuf_MODULE_COMPATIBLE TRUE)
  set(Protobuf_USE_STATIC_LIBS ON)
  find_package(Protobuf REQUIRED)
  message(STATUS "Using protobuf ${Protobuf_VERSION}")

  set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
  set(_REFLECTION gRPC::grpc++_reflection)
  if(CMAKE_CROSSCOMPILING)
    find_program(_PROTOBUF_PROTOC protoc)
  else()
    set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
  endif()

  # Find gRPC installation
  # Looks for gRPCConfig.cmake file installed by gRPC's cmake installation.
  find_package(gRPC CONFIG REQUIRED)
  message(STATUS "Using gRPC ${gRPC_VERSION}")

  set(_GRPC_GRPCPP gRPC::grpc++)
  if(CMAKE_CROSSCOMPILING)
    find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
  else()
    set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
  endif()
endif()

FetchContent_Declare(fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 10.2.1
)
FetchContent_MakeAvailable(fmt)

#set(FULFIL_LOG_DIR "/home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/logs")
#set(FULFIL_INI_DIR "/home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/configs")

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

#add_subdirectory("../Fulfil.MongoCpp" "build/FulfilMongoCpp")
#add_library(FulfilMongoCpp STATIC IMPORTED)
#set_property(TARGET FulfilMongoCpp PROPERTY
#             IMPORTED_LOCATION "Fulfil.MongoCpp")
add_subdirectory("../Fulfil.MongoCpp" build)
find_package(FulfilMongoCpp)

#add_library(FulfilMongoCpp STATIC IMPORTED)
#set_property(TARGET FulfilMongoCpp PROPERTY
#             IMPORTED_LOCATION "../Fulfil.MongoCpp/build")

#add_subdirectory(app)
add_subdirectory(src)
