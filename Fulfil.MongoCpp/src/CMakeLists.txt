
include_directories(include/FulfilMongoCpp)
include_directories(include/FulfilMongoCpp/mongo_filter)
include_directories(include/FulfilMongoCpp/mongo_json)
include_directories(include/FulfilMongoCpp/mongo_objects)
include_directories(include/FulfilMongoCpp/mongo_parse)

file(GLOB_RECURSE SOURCES "*.cpp")

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

add_library(FulfilMongoCpp STATIC ${SOURCES})

if (NOT FULFIL_DISABLE_JSON)
    target_link_libraries(FulfilMongoCpp LINK_PUBLIC json)
endif()


find_package(bson REQUIRED CONFIG NAMES bson-1.0)
#message("Found bson package ${bson}")
find_package(bsoncxx 3.6.6 REQUIRED)
find_package(mongocxx 3.6.6 REQUIRED)

#add_library(mongocxx 3.6.6 REQUIRED  CONFIG)

# Add Mongo dependency
# Get dynamic location of include directory
#get_target_property(BSON_IMPORT_PREFIX mongo::bson INTERFACE_INCLUDE_DIRECTORIES)
#get_target_property(MONGO_IMPORT_PREFIX mongo::mongoc INTERFACE_INCLUDE_DIRECTORIES)
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
