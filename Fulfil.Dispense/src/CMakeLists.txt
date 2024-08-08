# Define the library called "Fulfil.Dispense" and link all third party libraries needed by it

include_directories(include/Fulfil.Dispense)
include_directories(include/Fulfil.Dispense/bays)
include_directories(include/Fulfil.Dispense/commands)
include_directories(include/Fulfil.Dispense/commands/drop_target)
include_directories(include/Fulfil.Dispense/commands/item_edge_distance)
include_directories(include/Fulfil.Dispense/commands/nop)
include_directories(include/Fulfil.Dispense/commands/parsing)
include_directories(include/Fulfil.Dispense/commands/pre_side_dispense)
include_directories(include/Fulfil.Dispense/commands/post_drop)
include_directories(include/Fulfil.Dispense/commands/post_side_dispense)
include_directories(include/Fulfil.Dispense/commands/side_dispense_target)
include_directories(include/Fulfil.Dispense/commands/tray_validation)
include_directories(include/Fulfil.Dispense/commands/video)
include_directories(include/Fulfil.Dispense/dispense)
include_directories(include/Fulfil.Dispense/drop)
include_directories(include/Fulfil.Dispense/mongo)
include_directories(include/Fulfil.Dispense/recipes)
include_directories(include/Fulfil.Dispense/tray)
include_directories(include/Fulfil.Dispense/visualization)

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

