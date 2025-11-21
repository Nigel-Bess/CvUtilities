# Fulfil.MongoCpp/src/CMakeLists.txt

# Public include tree for this library
set(FULFIL_MONGOCPP_PUBLIC_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")

# Target-based includes
file(GLOB_RECURSE FULFIL_MONGOCPP_HEADERS
    "${FULFIL_MONGOCPP_PUBLIC_INCLUDE_DIR}/FulfilMongoCpp/*.h"
    "${FULFIL_MONGOCPP_PUBLIC_INCLUDE_DIR}/FulfilMongoCpp/*.hpp"
)

file(GLOB_RECURSE FULFIL_MONGOCPP_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
)

# Core library target
add_library(FulfilMongoCpp STATIC
    ${FULFIL_MONGOCPP_SOURCES}
    ${FULFIL_MONGOCPP_HEADERS}
)

set(_fulfil_mongocpp_lib_dir "${PROJECT_SOURCE_DIR}/build/lib")
set(_fulfil_mongocpp_bin_dir "${PROJECT_SOURCE_DIR}/build/bin")

set_target_properties(FulfilMongoCpp PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${_fulfil_mongocpp_lib_dir}"
    LIBRARY_OUTPUT_DIRECTORY "${_fulfil_mongocpp_lib_dir}"
    RUNTIME_OUTPUT_DIRECTORY "${_fulfil_mongocpp_bin_dir}"
    OUTPUT_NAME "FulfilMongoCpp"
)

# ===== Mongo C++ driver dependencies =====

# Get dynamic location of Mongo driver include dirs from the imported targets
get_target_property(BSON_IMPORT_PREFIX mongo::bsoncxx_shared INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(MONGO_IMPORT_PREFIX mongo::mongocxx_shared INTERFACE_INCLUDE_DIRECTORIES)

message("-- Including bsoncxx from ${BSON_IMPORT_PREFIX}")
message("-- Including mongocxx from ${MONGO_IMPORT_PREFIX}")

target_include_directories(FulfilMongoCpp
    PUBLIC
        "${FULFIL_MONGOCPP_PUBLIC_INCLUDE_DIR}"
        ${MONGO_IMPORT_PREFIX}
        ${BSON_IMPORT_PREFIX}
)

# Json feature flag
target_compile_definitions(FulfilMongoCpp
    PUBLIC
        DISABLE_JSON=${JSON_CODE}
)

# Link to mongo C++ driver + SSL
target_link_libraries(FulfilMongoCpp
    PRIVATE
        mongo::bsoncxx_shared
        mongo::mongocxx_shared
        ssl
        crypto
)

# Install rules
set(package FulfilMongoCpp)

install(TARGETS FulfilMongoCpp
        EXPORT FulfilMongoCppTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILE_SET HEADERS
)

set(FulfilMongoCpp_INSTALL_CMAKEDIR "${CMAKE_INSTALL_DATADIR}/${package}"
    CACHE PATH "CMake package config location relative to the install prefix"
)
mark_as_advanced(FulfilMongoCpp_INSTALL_CMAKEDIR)

install(FILES "../include/FulfilMongoCpp/mongo_connection.h" DESTINATION include)
