# CMake Version
cmake_minimum_required(VERSION 3.22)

# Project Name
project(tcs LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CACHE_VERSION 2)
set(FULFIL_LOG_DIR /code/mars/vimba)


find_package(OpenCV 4.6.0 REQUIRED)
find_package(Threads REQUIRED)

include_directories(include)
find_package(libmongoc-1.0 REQUIRED)

message("-- Found OpenCV lib ${OpenCV_VERSION}!")
include_directories(${OpenCV_INCLUDE_DIRS})

include(/home/fulfil/VimbaX_2024-1/api/examples/cmake/vmb_cmake_prefix_paths.cmake)
find_package(Vmb REQUIRED COMPONENTS CPP NAMES Vmb VmbC VmbCPP VmbImageTransform)
message(STATUS "Using Vmb ${Vmb_VERSION}")

set(OrbbecSDK_DIR "../Fulfil.CPPUtils/lib/orbbecsdk/lib")
find_package(OrbbecSDK REQUIRED)

link_directories(/home/fulfil/VimbaX_2024-1/api/lib)

add_subdirectory(../third-party ${CMAKE_CURRENT_BINARY_DIR}/libs/third-party)
add_subdirectory(../Fulfil.CPPUtils build)
#set(Fulfil.CPPUtils_DIR ../Fulfil.CPPUtils/build)
#find_package("Fulfil.CPPUtils" REQUIRED)

# Build test run executable
file(GLOB SOURCES "src/commands/**/*.cpp" "src/*.cpp" "src/**/*.cpp")
#add_executable(scripts "scripts/tcs_perception_scripts.cpp" ${SOURCES})
#target_link_libraries(scripts Fulfil.CPPUtils Vmb::CPP pthread json ${OpenCV_LIBS} )
#set_target_properties(scripts PROPERTIES CXX_STANDARD 17)

# TCS entrypoint
add_executable(tcs "tcs.cpp" ${SOURCES})
target_link_libraries(tcs Fulfil.CPPUtils Vmb::CPP ob::OrbbecSDK pthread json ${OpenCV_LIBS} )
set_target_properties(tcs PROPERTIES CXX_STANDARD 17)

# TCS eval entrypoint
add_executable(eval "eval.cpp" ${SOURCES})
target_link_libraries(eval Fulfil.CPPUtils Vmb::CPP ob::OrbbecSDK pthread json ${OpenCV_LIBS} )
set_target_properties(eval PROPERTIES CXX_STANDARD 17)

# PBL entrypoint
add_executable(pbl "pbl.cpp" ${SOURCES})
target_link_libraries(pbl Fulfil.CPPUtils Vmb::CPP ob::OrbbecSDK pthread json ${OpenCV_LIBS} )
set_target_properties(pbl PROPERTIES CXX_STANDARD 17)

# Orbbec playground entrypoint
add_executable(orbbec "orbbec.cpp" ${SOURCES})
target_link_libraries(orbbec Fulfil.CPPUtils Vmb::CPP ob::OrbbecSDK pthread json ${OpenCV_LIBS} )
set_target_properties(orbbec PROPERTIES CXX_STANDARD 17)

# Test stuff
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# TODO!!! Restore this but there's a weird mongo lib docker error...
FetchContent_MakeAvailable(googletest)
enable_testing()

# Unit tests
add_executable(
  unit_test ${SOURCES} "test/unit_tests.cpp"
)
target_link_libraries(
    unit_test GTest::gtest_main Fulfil.CPPUtils Vmb::CPP pthread json ${OpenCV_LIBS}
)
set_target_properties(unit_test PROPERTIES CXX_STANDARD 17)

# Integration tests
add_executable(
  int_test ${SOURCES} "test/int_tests.cpp"
)
target_link_libraries(
    int_test GTest::gtest_main Fulfil.CPPUtils Vmb::CPP pthread json ${OpenCV_LIBS}
)
set_target_properties(int_test PROPERTIES CXX_STANDARD 17)

include(GoogleTest)
gtest_discover_tests(int_test)
