cmake_minimum_required(VERSION 3.18)
cmake_policy(VERSION 3.18...3.22)

project(Fulfil.DepthCam
        VERSION 3.0
        DESCRIPTION "A library for depth cam code"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
find_package(Threads REQUIRED)

set(INCLUDE_INSTALL_DIR include/ )
set(LIB_INSTALL_DIR lib/ )
set(SYSCONFIG_INSTALL_DIR lib/Fulfil.Dispense/ )

# Root of the CV repo (Fulfil.ComputerVision)
set(CV_ROOT "${PROJECT_SOURCE_DIR}/..")

# Using prebuilt Fulfil.CPPUtils static library
add_library(Fulfil.CPPUtils STATIC IMPORTED)
set_target_properties(Fulfil.CPPUtils PROPERTIES
    IMPORTED_LOCATION
        "${CV_ROOT}/Fulfil.CPPUtils/build/lib/libFulfil.CPPUtils.a"
    INTERFACE_INCLUDE_DIRECTORIES
        "${CV_ROOT}/Fulfil.CPPUtils/include"
)

# Using prebuilt Fulfil.OrbbecUtils static library
add_library(Fulfil.OrbbecUtils STATIC IMPORTED)
set_target_properties(Fulfil.OrbbecUtils PROPERTIES
    IMPORTED_LOCATION
        "${CV_ROOT}/Fulfil.OrbbecUtils/build/lib/libFulfil.OrbbecUtils.a"
    INTERFACE_INCLUDE_DIRECTORIES
        "${CV_ROOT}/Fulfil.OrbbecUtils/include"
)

include(CMakePackageConfigHelpers)
configure_package_config_file(CMakeLists.txt.in
  ${CMAKE_CURRENT_BINARY_DIR}/Fulfil.DispenseConfig.cmake
  INSTALL_DESTINATION ${LIB_INSTALL_DIR}/Fulfil.Dispense/cmake
  PATH_VARS INCLUDE_INSTALL_DIR SYSCONFIG_INSTALL_DIR)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/Fulfil.DispenseConfigVersion.cmake
  VERSION 1.0.0
  COMPATIBILITY SameMajorVersion )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Fulfil.DispenseConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/Fulfil.DispenseConfigVersion.cmake
        DESTINATION ${LIB_INSTALL_DIR}/Fulfil.Dispense/cmake )

if(DEFINED IS_CONTAINERIZED)
  add_subdirectory(libs/librealsense)
endif()


add_subdirectory(app)
add_subdirectory(src)
