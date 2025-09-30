# Define the library called "Fulfil.Dispense" and link all third party libraries needed by it

file(GLOB_RECURSE SUBDIRS LIST_DIRECTORIES true "include/Fulfil.Dispense/*")
include_directories(include/Fulfil.Dispense ${SUBDIRS})

file(GLOB_RECURSE SOURCES "*.cpp")

add_library(Fulfil.Dispense STATIC ${SOURCES})

target_link_libraries(Fulfil.Dispense LINK_PUBLIC json)

target_include_directories(Fulfil.Dispense PUBLIC ${PROJECT_SOURCE_DIR}/include)

option(ALLWARNINGS OFF)
IF(ALLWARNINGS)
    message("\n*** Prepare yourself. All warnings enabled ***\n")
    target_compile_options(Fulfil.Dispense PRIVATE
        -Wall -Wextra -Wextra-semi -Wmisleading-indentation
        -Wunused -Wnull-dereference #-Wshadow
        -Wuseless-cast #-Wold-style-cast -Wcast-align
        # -Wdouble-promotion -Wconversion -Wsign-conversion
        # -Wduplicated-cond -Wduplicated-branches  -Wlogical-op -Wimplicit-fallthrough
        # -Wpedantic -Wformat=2
        #  -Wnon-virtual-dtor -Woverloaded-virtual
    )
endif()
target_link_libraries(Fulfil.Dispense
        PRIVATE Fulfil.DepthCam Fulfil.CPPUtils FulfilMongoCpp stdc++fs ${_REFLECTION} ${_GRPC_GRPCPP} ${_PROTOBUF_LIBPROTOBUF})



