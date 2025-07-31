# Setup the project

# Specify a minimum version of CMake
cmake_minimum_required(VERSION 3.22)
cmake_policy(VERSION 3.22)


# Project name and settings
project(
        FulfilMongoCpp
        VERSION 2.0
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
find_package(Threads REQUIRED)

set(INCLUDE_INSTALL_DIR include/ )
set(LIB_INSTALL_DIR lib/ )
set(SYSCONFIG_INSTALL_DIR lib/FulfilMongoCpp/ )
include(CMakePackageConfigHelpers)
configure_package_config_file(CMakeLists.txt.in
  ${CMAKE_CURRENT_BINARY_DIR}/FulfilMongoCppConfig.cmake
  INSTALL_DESTINATION ${LIB_INSTALL_DIR}/FulfilMongoCpp/cmake
  PATH_VARS INCLUDE_INSTALL_DIR SYSCONFIG_INSTALL_DIR)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/FulfilMongoCppConfigVersion.cmake
  VERSION 1.0.0
  COMPATIBILITY SameMajorVersion )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/FulfilMongoCppConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/FulfilMongoCppConfigVersion.cmake
        DESTINATION ${LIB_INSTALL_DIR}/FulfilMongoCpp/cmake )

set(CMAKE_BUILD_TYPE Release)
# Allow for Macro definition on cmake command line. Default is build root on current system.
option(FULFIL_DISABLE_JSON "Will not build nlohmann::json and MongoJsonDocument." OFF)

if (FULFIL_DISABLE_JSON)
    message("-- Disabling build of MongoJson functionality")
    set(MONGO_JSON_LIST "")
    set(JSON_CODE 1)
else()
    message("-- Including build of Json functionality")
    #set(MONGO_JSON_LIST "${PROJECT_SOURCE_DIR}/include/FulfilMongoCpp/mongo_json/mongo_json_document.h;${PROJECT_SOURCE_DIR}/include/FulfilMongoCpp/mongo_json/json.hpp;mongo_json/mongo_json_document.cpp")
    set(MONGO_JSON_LIST "${PROJECT_SOURCE_DIR}/include/FulfilMongoCpp/mongo_json/mongo_json_document.h;mongo_json/mongo_json_document.cpp")
    set(JSON_CODE 0)
endif()

find_package(bson REQUIRED CONFIG NAMES bson-1.0)
message("Found bson package ${bson}")
find_package(bsoncxx REQUIRED)
find_package(mongocxx REQUIRED)

add_subdirectory(app)
add_subdirectory(src)
