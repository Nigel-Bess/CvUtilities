# Define the library called "Fulfil.CPPUtils" and link all third party libraries needed by it

set(PACKAGE_VERSION "1.0.0")
include(CMakePackageConfigHelpers)
include_directories(include/Fulfil.CPPUitls)
include_directories(include/Fulfil.CPPUitls/comm/)
include_directories(include/Fulfil.CPPUitls/eigen)
include_directories(include/Fulfil.CPPUitls/networking)
include_directories(include/Fulfil.CPPUitls/processing_queue)
include_directories(include/Fulfil.CPPUitls/inih)
file(GLOB SOURCES "*.cpp" "comm/*.cc" "networking/*.cpp" "eigen/*.cpp" "comm/*.h")

add_library(Fulfil.CPPUtils STATIC ${SOURCES})

find_package(CURL REQUIRED)
target_compile_definitions(Fulfil.CPPUtils PUBLIC DEFAULT_FULFIL_LOGGING_DIR="${FULFIL_LOG_DIR}")
target_compile_definitions(Fulfil.CPPUtils PUBLIC DEFAULT_FULFIL_INI_DIR="${FULFIL_INI_DIR}")


# Include the header files and make PUBLIC so can be accessed by code using this library
target_include_directories(Fulfil.CPPUtils PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(Fulfil.CPPUtils PUBLIC ${PROJECT_SOURCE_DIR}/src/comm)
target_include_directories(Fulfil.CPPUtils SYSTEM INTERFACE ${PROJECT_SOURCE_DIR}/include)
target_include_directories(Fulfil.CPPUtils SYSTEM INTERFACE ${PROJECT_SOURCE_DIR}/src/comm)
target_link_libraries(Fulfil.CPPUtils ${CURL_LIBRARIES} ${_REFLECTION} ${_GRPC_GRPCPP} ${_PROTOBUF_LIBPROTOBUF} FulfilMongoCpp)

