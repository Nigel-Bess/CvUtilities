# Define the library called "Fulfil.MongoCpp" and link all third party libraries needed by it
file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/FulfilMongoCpp/*.h")
file(GLOB MONGO_OBJ_HEADER_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/FulfilMongoCpp/mongo_objects/*.h")
file(GLOB MONGO_FILTER_HEADER_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/FulfilMongoCpp/mongo_filter/*.h")
#file(GLOB MONGO_FILTER_HEADER_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/Fulfil.MongoCpp/mongo_machine/*.h")
file(GLOB MONGO_PARSE_HEADER_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/FulfilMongoCpp/mongo_parse/*.h")

# Find mongo packages (assumes that user used global install procedure in README)


if (FULFIL_DISABLE_JSON)
    message("-- Disabling build of MongoJson functionality")
    set(MONGO_JSON_LIST "")
    set(JSON_CODE 1)
else()
    message("-- Including build of Json functionality")
    set(MONGO_JSON_LIST "${PROJECT_SOURCE_DIR}/include/FulfilMongoCpp/mongo_json/mongo_json_document.h;${PROJECT_SOURCE_DIR}/include/FulfilMongoCpp/mongo_json/json.hpp;mongo_json/mongo_json_document.cpp")
    #set(MONGO_JSON_LIST "${PROJECT_SOURCE_DIR}/include/FulfilMongoCpp/mongo_json/mongo_json_document.h;mongo_json/mongo_json_document.cpp")
    set(JSON_CODE 0)
endif()

add_library(FulfilMongoCpp STATIC
        ${HEADER_LIST}
        ${MONGO_OBJ_HEADER_LIST}
        ${MONGO_FILTER_HEADER_LIST}
        ${MONGO_PARSE_HEADER_LIST}
        ${MONGO_JSON_LIST}
        base_mongo_connection.cpp
        mongo_connection.cpp
        mongo_filter/mongo_filters.cpp
        mongo_objects/mongo_basic_document.cpp
        #mongo_objects/mongo_scoped_document.cpp
        #mongo_machine/mongo_machine.cpp
        mongo_objects/mongo_element.cpp
        mongo_objects/mongo_document_element.cpp
        mongo_objects/mongo_array_element.cpp
        mongo_parse/mongo_bsonxx_parser.cpp
        mongo_parse/mongo_bsonxx_encoder.cpp
        mongo_objects/mongo_object_id.cpp)


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
