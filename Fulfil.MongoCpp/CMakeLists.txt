# Setup the project

# Specify a minimum version of CMake
cmake_minimum_required(VERSION 3.18)
cmake_policy(VERSION 3.18)


# Project name and settings
project(
        FulfilMongoCpp
        VERSION 2.0
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_BUILD_TYPE Debug)
# Allow for Macro definition on cmake command line. Default is build root on current system.
option(FULFIL_DISABLE_JSON "Will not build nlohmann::json and MongoJsonDocument." OFF)

add_subdirectory(app)
add_subdirectory(src)

