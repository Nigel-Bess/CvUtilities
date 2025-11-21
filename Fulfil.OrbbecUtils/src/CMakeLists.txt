# Fulfil.OrbbecUtils/src/CMakeLists.txt

set(PACKAGE_VERSION "1.0.0")
include(CMakePackageConfigHelpers)

# Public include root
set(FULFIL_ORBBEC_PUBLIC_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")

# Collect headers and sources
file(GLOB_RECURSE FULFIL_ORBBEC_HEADERS
    "${FULFIL_ORBBEC_PUBLIC_INCLUDE_DIR}/Fulfil.OrbbecUtils/*.h"
    "${FULFIL_ORBBEC_PUBLIC_INCLUDE_DIR}/Fulfil.OrbbecUtils/*.hpp"
)

file(GLOB_RECURSE FULFIL_ORBBEC_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
)

# Core library target
add_library(Fulfil.OrbbecUtils STATIC
    ${FULFIL_ORBBEC_SOURCES}
    ${FULFIL_ORBBEC_HEADERS}
)

set(_fulfil_orbbec_lib_dir "${PROJECT_SOURCE_DIR}/build/lib")
set(_fulfil_orbbec_bin_dir "${PROJECT_SOURCE_DIR}/build/bin")

set_target_properties(Fulfil.OrbbecUtils PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${_fulfil_orbbec_lib_dir}"
    LIBRARY_OUTPUT_DIRECTORY "${_fulfil_orbbec_lib_dir}"
    RUNTIME_OUTPUT_DIRECTORY "${_fulfil_orbbec_bin_dir}"
    OUTPUT_NAME "Fulfil.OrbbecUtils"
)

# Default log + ini dirs from top-level CMakeLists
target_compile_definitions(Fulfil.OrbbecUtils
    PUBLIC
        DEFAULT_FULFIL_LOGGING_DIR="${FULFIL_LOG_DIR}"
        DEFAULT_FULFIL_INI_DIR="${FULFIL_INI_DIR}"
)

# Public include dirs for users of this library
target_include_directories(Fulfil.OrbbecUtils
    PUBLIC
        "${FULFIL_ORBBEC_PUBLIC_INCLUDE_DIR}"
        "${PROJECT_SOURCE_DIR}/src/comm"
        ${OpenCV_INCLUDE_DIRS}
)

# Link third-party deps + Orbbec SDK + prebuilt MongoCpp
find_package(CURL REQUIRED)

target_link_libraries(Fulfil.OrbbecUtils
    PRIVATE
        ${CURL_LIBRARIES}
        ob::OrbbecSDK
        ${_REFLECTION}
        ${_GRPC_GRPCPP}
        ${_PROTOBUF_LIBPROTOBUF}
        FulfilMongoCpp
)
