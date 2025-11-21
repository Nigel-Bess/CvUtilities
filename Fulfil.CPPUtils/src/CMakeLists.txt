# Fulfil.CPPUtils/src/CMakeLists.txt

set(PACKAGE_VERSION "1.0.0")
include(CMakePackageConfigHelpers)

# Public include root
set(FULFIL_CPPUTILS_PUBLIC_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")

# Collect sources and headers
file(GLOB_RECURSE FULFIL_CPPUTILS_HEADERS
    "${FULFIL_CPPUTILS_PUBLIC_INCLUDE_DIR}/Fulfil.CPPUtils/*.h"
    "${FULFIL_CPPUTILS_PUBLIC_INCLUDE_DIR}/Fulfil.CPPUtils/*.hpp"
)

file(GLOB_RECURSE FULFIL_CPPUTILS_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
)

# Core library target
add_library(Fulfil.CPPUtils STATIC
    ${FULFIL_CPPUTILS_SOURCES}
    ${FULFIL_CPPUTILS_HEADERS}
)

set(_fulfil_cpputils_lib_dir "${PROJECT_SOURCE_DIR}/build/lib")
set(_fulfil_cpputils_bin_dir "${PROJECT_SOURCE_DIR}/build/bin")

set_target_properties(Fulfil.CPPUtils PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${_fulfil_cpputils_lib_dir}"
    LIBRARY_OUTPUT_DIRECTORY "${_fulfil_cpputils_lib_dir}"
    RUNTIME_OUTPUT_DIRECTORY "${_fulfil_cpputils_bin_dir}"
    OUTPUT_NAME "Fulfil.CPPUtils"
)

# Default log + ini dirs are computed in the top-level CMakeLists
target_compile_definitions(Fulfil.CPPUtils
    PUBLIC
        DEFAULT_FULFIL_LOGGING_DIR="${FULFIL_LOG_DIR}"
        DEFAULT_FULFIL_INI_DIR="${FULFIL_INI_DIR}"
)

# Public include dirs for any user of this library
target_include_directories(Fulfil.CPPUtils
    PUBLIC
        "${FULFIL_CPPUTILS_PUBLIC_INCLUDE_DIR}"
        "${PROJECT_SOURCE_DIR}/src/comm"
        ${OpenCV_INCLUDE_DIRS}
)

# Link third-party deps + prebuilt MongoCpp library
find_package(CURL REQUIRED)

target_link_libraries(Fulfil.CPPUtils
    PRIVATE
        ${CURL_LIBRARIES}
        ${_REFLECTION}
        ${_GRPC_GRPCPP}
        ${_PROTOBUF_LIBPROTOBUF}
        FulfilMongoCpp
)

