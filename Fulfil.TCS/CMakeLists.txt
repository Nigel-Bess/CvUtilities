# CMake Version
cmake_minimum_required(VERSION 3.22)

# Project Name
project(allied-vision LANGUAGES CXX)

set(CMAKE_CACHE_VERSION 2)
set(FULFIL_LOG_DIR /code/mars/vimba)
include_directories(include)
include_directories(include/aruco)
include_directories(include/commands)

find_package(Threads REQUIRED)
find_package(OpenCV 4.6.0 REQUIRED)

message("-- Found OpenCV lib ${OpenCV_VERSION}!")
include_directories(${OpenCV_INCLUDE_DIRS})

include(/home/fulfil/VimbaX_2024-1/api/examples/cmake/vmb_cmake_prefix_paths.cmake)
find_package(Vmb REQUIRED COMPONENTS CPP NAMES Vmb VmbC VmbCPP VmbImageTransform)
message(STATUS "Using Vmb ${Vmb_VERSION}")

link_directories(/home/fulfil/VimbaX_2024-1/api/lib)

add_subdirectory(../Fulfil.CPPUtils ${CMAKE_CURRENT_BINARY_DIR}/libs/Fulfil.CPPUtils)
add_subdirectory(../third-party ${CMAKE_CURRENT_BINARY_DIR}/libs/third-party)

# Build test run executable
file(GLOB TEST_SOURCES "src/*.cpp" "src/aruco/*.cpp" "src/entrypoints/tcs_perception_test.cpp" "src/commands/*.cpp")
add_executable(scripts ${TEST_SOURCES})
target_link_libraries(scripts Fulfil.CPPUtils Vmb::CPP pthread json ${OpenCV_LIBS} )
set_target_properties(scripts PROPERTIES CXX_STANDARD 17)

# Build main executable, main==prod and you'd know that if you're really from Fulfil
file(GLOB SOURCES "src/*.cpp" "src/aruco/*.cpp" "src/entrypoints/main.cpp" "src/commands/*.cpp")
add_executable(main ${SOURCES})
target_link_libraries(main Fulfil.CPPUtils Vmb::CPP pthread json ${OpenCV_LIBS} )
set_target_properties(main PROPERTIES CXX_STANDARD 17)


# Test stuff
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
enable_testing()
add_executable(
  unit_test
  "src/entrypoints/tests.cpp"
)
target_link_libraries(
    unit_test
  GTest::gtest_main
)
include(GoogleTest)
gtest_discover_tests(unit_test)
