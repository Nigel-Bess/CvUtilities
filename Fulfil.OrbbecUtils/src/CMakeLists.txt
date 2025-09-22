# Define the library called "Fulfil.OrbbecUtils" and link all third party libraries needed by it

set(PACKAGE_VERSION "1.0.0")
include(CMakePackageConfigHelpers)
include_directories(include/Fulfil.CPPUitls)
include_directories(include/Fulfil.CPPUitls/*)
file(GLOB SOURCES "*.cpp" "*.h" "*.cc" "*.cpp **/*.cpp" "**/*.h" "**/*.cc" "**/*.cpp")

add_library(Fulfil.OrbbecUtils STATIC ${SOURCES})

find_package(CURL REQUIRED)
target_compile_definitions(Fulfil.OrbbecUtils PUBLIC DEFAULT_FULFIL_LOGGING_DIR="${FULFIL_LOG_DIR}")
target_compile_definitions(Fulfil.OrbbecUtils PUBLIC DEFAULT_FULFIL_INI_DIR="${FULFIL_INI_DIR}")


# Include the header files and make PUBLIC so can be accessed by code using this library
target_include_directories(Fulfil.OrbbecUtils PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(Fulfil.OrbbecUtils PUBLIC ${PROJECT_SOURCE_DIR}/src/comm)
target_include_directories(Fulfil.OrbbecUtils SYSTEM INTERFACE ${PROJECT_SOURCE_DIR}/include)
target_include_directories(Fulfil.OrbbecUtils SYSTEM INTERFACE ${PROJECT_SOURCE_DIR}/src/comm)
target_link_libraries(Fulfil.OrbbecUtils ${CURL_LIBRARIES}  ob::OrbbecSDK ${_REFLECTION} ${_GRPC_GRPCPP} ${_PROTOBUF_LIBPROTOBUF} FulfilMongoCpp)

