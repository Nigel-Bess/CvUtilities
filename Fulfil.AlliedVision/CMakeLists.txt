# CMake Version
cmake_minimum_required(VERSION 3.22)

# Project Name
project(allied-vision LANGUAGES CXX)

set(FULFIL_LOG_DIR /code/mars/vimba)
include_directories(include)
include_directories(include/commands/bag_release)

file(GLOB SOURCES "src/*.cpp" "src/commands/bag_release/*.cpp")

find_package(Threads REQUIRED)
find_package(OpenCV 4.5.5 REQUIRED)

message("-- Found OpenCV lib ${OpenCV_VERSION}!")
include_directories(${OpenCV_INCLUDE_DIRS})

include(/home/fulfil/VimbaX_2024-1/api/examples/cmake/vmb_cmake_prefix_paths.cmake)
find_package(Vmb REQUIRED COMPONENTS CPP NAMES Vmb VmbC VmbCPP VmbImageTransform)
message(STATUS "Using Vmb ${Vmb_VERSION}")

link_directories(/home/fulfil/VimbaX_2024-1/api/lib)

add_subdirectory(../Fulfil.CPPUtils ${CMAKE_CURRENT_BINARY_DIR}/libs/Fulfil.CPPUtils)
add_subdirectory(../third-party ${CMAKE_CURRENT_BINARY_DIR}/libs/third-party)
add_executable(stark_test ${SOURCES})
target_link_libraries(stark_test Fulfil.CPPUtils Vmb::CPP pthread json ${OpenCV_LIBS} )
set_target_properties(stark_test PROPERTIES CXX_STANDARD 17)



