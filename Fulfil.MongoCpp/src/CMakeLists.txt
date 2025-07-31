
include_directories(include/FulfilMongoCpp)
include_directories(include/FulfilMongoCpp/mongo_filter)
include_directories(include/FulfilMongoCpp/mongo_json)
include_directories(include/FulfilMongoCpp/mongo_objects)
include_directories(include/FulfilMongoCpp/mongo_parse)

file(GLOB_RECURSE SOURCES "*.cpp" "*.hpp")

add_library(FulfilMongoCpp STATIC ${SOURCES})

if (NOT FULFIL_DISABLE_JSON)
    target_link_libraries(FulfilMongoCpp LINK_PUBLIC)
endif()

target_sources(FulfilMongoCpp PUBLIC ${SOURCES})
#add_library(mongocxx 3.6.6 REQUIRED  CONFIG)

# Add Mongo dependency
# Get dynamic location of include directory
get_target_property(BSON_IMPORT_PREFIX mongo::bsoncxx_shared INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(MONGO_IMPORT_PREFIX mongo::mongocxx_shared INTERFACE_INCLUDE_DIRECTORIES)

message("-- Including bsoncxx from ${BSON_IMPORT_PREFIX}")
message("-- Including mongocxx from ${MONGO_IMPORT_PREFIX}")

# Add Mongo dependency
target_include_directories(
        FulfilMongoCpp PUBLIC
        ${MONGO_IMPORT_PREFIX}
        ${BSON_IMPORT_PREFIX})

target_compile_definitions(FulfilMongoCpp PUBLIC DISABLE_JSON=${JSON_CODE})
unset(JSON_CODE CACHE)

# Create imported target mongo::mongocxx_static
target_include_directories(FulfilMongoCpp PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(FulfilMongoCpp PRIVATE mongo::bsoncxx_shared mongo::mongocxx_shared ssl crypto)

set(package FulfilMongoCpp)

install(TARGETS FulfilMongoCpp
        EXPORT FulfilMongoCppTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILE_SET HEADERS # NOTE: requierd from CMake to install the FILE_SET HEADERS again!
)

# Allow package maintainers to freely override the path for the configs
set(FulfilMongoCpp_INSTALL_CMAKEDIR "${CMAKE_INSTALL_DATADIR}/${package}"
    CACHE PATH "CMake package config location relative to the install prefix"
)
mark_as_advanced(FulfilMongoCpp_INSTALL_CMAKEDIR)

install(FILES "../include/FulfilMongoCpp/mongo_connection.h" DESTINATION include)
