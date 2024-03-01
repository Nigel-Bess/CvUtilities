# Define the library called "Fulfil.Dispense" and link all third party libraries needed by it

# include_directories(include/Fulfil.Dispense)
# include_directories(include/Fulfil.Dispense/drop)
# include_directories(include/Fulfil.Dispense/bays)
# include_directories(include/Fulfil.Dispense/tray/)
# include_directories(include/Fulfil.CPPUitls/inih)
# file(GLOB SOURCES "*.cpp" "comm/*.cc" "networking/*.cpp" "eigen/*.cpp" "comm/*.h")

# add_library(Fulfil.CPPUtils STATIC ${SOURCES})

file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/Fulfil.Dispense/*.h")
file(GLOB DROP_HEADER_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/Fulfil.Dispense/drop/*.h")
file(GLOB BAYS_HEADER_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/Fulfil.Dispense/bays/*.h")
file(GLOB BAYS_HEADER_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/Fulfil.Dispense/tray/*.h")
file(GLOB COMMANDS_HEADER_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/Fulfil.Dispense/commands/*.h")
file(GLOB MOTION_HEADER_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/Fulfil.Dispense/motion/*.h")
file(GLOB DROP_TARGET_COMMANDS_HEADER_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/Fulfil.Dispense/commands/drop_target/*.h")
file(GLOB PRE_DROP_BULK_COMMANDS_HEADER_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/Fulfil.Dispense/commands/pre_drop_bulk/*.h")
file(GLOB POST_DROP_COMMANDS_HEADER_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/Fulfil.Dispense/commands/post_drop/*.h")
file(GLOB PARSING_COMMANDS_HEADER_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/Fulfil.Dispense/commands/parsing/*.h")
file(GLOB NOP_COMMANDS_HEADER_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/Fulfil.Dispense/commands/nop/*.h")
file(GLOB STOP_COMMANDS_HEADER_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/Fulfil.Dispense/commands/stop/*.h")
file(GLOB STOP_COMMANDS_HEADER_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/Fulfil.Dispense/commands/tray_validation/*.h")
file(GLOB DROP_VISUALIZATION_HEADER_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/Fulfil.Dispense/visualization/*.h")
file(GLOB MONGO_HEADER_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/Fulfil.Dispense/mongo/*.h")
file(GLOB DISPENSE_IMAGE_PERSISTENCE_HEADER_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/Fulfil.Dispense/dispense/image_persistence/*.h")

add_library(Fulfil.Dispense STATIC ${HEADER_LIST}
        ${DROP_HEADER_LIST}
        ${BAYS_HEADER_LIST}
        ${COMMANDS_HEADER_LIST}
        ${MOTION_HEADER_LIST}
        ${DROP_TARGET_COMMANDS_HEADER_LIST}
        ${PRE_DROP_BULK_COMMANDS_HEADER_LIST}
        ${PARSING_COMMANDS_HEADER_LIST}
        ${NOP_COMMANDS_HEADER_LIST}
        ${STOP_COMMANDS_HEADER_LIST}
        ${MONGO_HEADER_LIST}
        commands/nop/nop_request.cpp
        commands/nop/nop_response.cpp
        commands/parsing/dispense_json_parser.cpp
        commands/parsing/dispense_request_parser.cpp
        commands/drop_target/json_item.cpp
        commands/drop_target/drop_target_details.cpp
        commands/drop_target/drop_target_request.cpp
        commands/drop_target/drop_target_response.cpp
        commands/item_edge_distance/item_edge_distance_request.cpp
        commands/item_edge_distance/item_edge_distance_response.cpp
        commands/pre_LFR_request.cpp
        commands/post_drop/post_LFR_request.cpp
        commands/start_lfb_video_request.cpp
        commands/start_tray_video_request.cpp
        commands/code_response.cpp
        commands/stop_lfb_video_request.cpp
        commands/stop_tray_video_request.cpp
        commands/tray_validation/tray_validation_response.cpp
        commands/tray_validation/tray_validation_request.cpp
        commands/tray_validation/tray_validation_details.cpp
        commands/update_state_request.cpp
        commands/dispense_response.cpp
        commands/home_motor_request.cpp
        commands/position_motor_request.cpp
        commands/error_response.cpp
        commands/content_response.cpp
        dispense/dispense_manager.cpp
        dispense/dispense_processing_queue_predicate.cpp
        dispense/realsense_bay_parser.cpp
        dispense/realsense_manager.cpp
        dispense/realsense_runner_factory.cpp
        dispense/test_manager.cpp
        drop/drop_manager.cpp
        drop/drop_result.cpp
        drop/drop_zone_searcher.cpp
        drop/pre_post_compare.cpp
        drop/drop_grid.cpp
        tray/tray_algorithm.cpp
        tray/tray.cpp
        tray/tray_result.cpp
        tray/item_edge_distance_result.cpp
        motion/trinamic.cpp
        motion/serial.cpp
        motion/motion_timer.cpp
        mongo/mongo_bag_state.cpp
        mongo/mongo_tray_calibration.cpp
        visualization/live_viewer.cpp
        dispense/image_persistence/realsense_image_persistence_manager.cpp
        dispense/image_persistence/realsense_timestamper.cpp ../include/Fulfil.Dispense/dispense/image_persistence/dispense_file_manager.h dispense/image_persistence/realsense_file_manager.cpp ../include/Fulfil.Dispense/dispense/image_persistence/realsense_file_manager.h commands/parsing/command_parsing_errors.cpp ../include/Fulfil.Dispense/commands/parsing/command_parsing_errors.h ../include/Fulfil.Dispense/bays.h ../include/Fulfil.Dispense/commands.h ../include/Fulfil.Dispense/dispense.h tray/tray_lane.cpp ../include/Fulfil.Dispense/tray/tray_lane.h commands/get_state_request.cpp ../include/Fulfil.Dispense/commands/get_state_request.h ../include/Fulfil.Dispense/commands/pre_LFR_request.h ../include/Fulfil.Dispense/commands/update_state_request.h mongo/mongo_bag_state.cpp ../include/Fulfil.Dispense/mongo/mongo_bag_state.h commands/post_drop/post_LFR_response.cpp ../include/Fulfil.Dispense/commands/post_drop/post_LFR_response.h mongo/packing_state.cpp ../include/Fulfil.Dispense/mongo/packing_state.h
        ../include/Fulfil.Dispense/tray/tray_parser.h
        ../include/Fulfil.Dispense/tray/mongo_json_utils.h
        tray/mongo_json_utils.cpp tray/tray_parser.cpp
        tray/measurement_helpers.cpp
        visualization/make_media.cpp tray/tray_manager.cpp
        ../include/Fulfil.Dispense/recipes/tray_dimensional_configuration.h
        ../include/Fulfil.Dispense/recipes/visualization_presets.h
        ../include/Fulfil.Dispense/recipes/tray_item_distance_calculation_settings.h
        recipes/tray_dimensional_configuration.cpp recipes/tray_calibration_manager.cpp recipes/tray_item_distance_calculation_settings.cpp tray/tray_process_logging_messages.cpp
        dispense/drop_error_codes.cpp)



target_include_directories(Fulfil.Dispense PUBLIC ${PROJECT_SOURCE_DIR}/include)

option(HATEYOURSELF OFF)
IF(HATEYOURSELF)
    message("\n*** Prepare yourself. All warnings enabled. Prepare to drown ***\n")
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

