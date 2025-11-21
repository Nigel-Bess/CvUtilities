# Fulfil.Dispense/src/CMakeLists.txt

# Headers & sources
file(GLOB_RECURSE SUBDIRS LIST_DIRECTORIES true "include/Fulfil.Dispense/*")
include_directories(include/Fulfil.Dispense ${SUBDIRS})

file(GLOB_RECURSE SOURCES "*.cpp")

add_library(Fulfil.Dispense STATIC ${SOURCES})

# json target from third-party
target_link_libraries(Fulfil.Dispense LINK_PUBLIC json)

# Public includes for Dispense itself
target_include_directories(Fulfil.Dispense
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

# Default INI dir (can be overridden via -DFULFIL_INI_DIR=... at configure time)
if(NOT DEFINED FULFIL_INI_DIR)
    set(FULFIL_INI_DIR "/home/fulfil/config")
endif()

target_compile_definitions(Fulfil.Dispense
    PUBLIC
        DEFAULT_FULFIL_INI_DIR="${FULFIL_INI_DIR}"
)

# Pull in includes from CPPUtils, MongoCpp, DepthCam and mongo-cxx/OpenCV
cmake_path(GET CMAKE_SOURCE_DIR PARENT_PATH COMPUTER_VISION_MODULES_PARENT_PATH)

target_include_directories(Fulfil.Dispense
    PUBLIC
        "${COMPUTER_VISION_MODULES_PARENT_PATH}/Fulfil.CPPUtils/include"
        "${COMPUTER_VISION_MODULES_PARENT_PATH}/Fulfil.MongoCpp/include"
        "${COMPUTER_VISION_MODULES_PARENT_PATH}/Fulfil.DepthCam/include"
        ${OpenCV_INCLUDE_DIRS}
        ${MONGO_IMPORT_PREFIX}
        ${BSON_IMPORT_PREFIX}
)

# Link to DepthCam + CPPUtils + MongoCpp + protobuf/gRPC
option(ALLWARNINGS OFF)
IF(ALLWARNINGS)
    message("\n*** Prepare yourself. All warnings enabled ***\n")
    target_compile_options(Fulfil.Dispense PRIVATE
        -Wall -Wextra -Wextra-semi -Wmisleading-indentation
        -Wunused -Wnull-dereference
        -Wuseless-cast
    )
endif()

target_link_libraries(Fulfil.Dispense
    PRIVATE
        Fulfil.DepthCam
        Fulfil.CPPUtils
        FulfilMongoCpp
        stdc++fs
        ${_REFLECTION}
        ${_GRPC_GRPCPP}
        ${_PROTOBUF_LIBPROTOBUF}
        ${CURL_LIBRARIES}
)
