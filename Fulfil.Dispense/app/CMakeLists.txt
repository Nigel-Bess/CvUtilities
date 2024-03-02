


set(DIV_SECTION "===================")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

if(EXISTS ${COMPUTER_VISION_MODULES_PARENT_PATH}/.git)
    message("\nFound valid .git directory. Generating version info.")
    execute_process(COMMAND git rev-parse --short HEAD
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            OUTPUT_VARIABLE DISPENSE_COMMIT
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND bash -c "eval $(git for-each-ref --count 1 --merged=HEAD --sort=-committerdate --shell --format='printf %(refname:short); ' refs/tags)"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            OUTPUT_VARIABLE LATEST_GIT_TAG)
else()
    message(WARNING "\nDid not find valid .git directory in ${COMPUTER_VISION_MODULES_PARENT_PATH}.")
    set(LATEST_GIT_TAG  "Unknown")
    set(DISPENSE_COMMIT "Unknown")
endif()
execute_process(COMMAND date -u "+%b. %0d, %Y at %H:%M:%S %Z"
OUTPUT_VARIABLE BUILD_DATE
OUTPUT_STRIP_TRAILING_WHITESPACE)



# ----------------------------------------------------------------------------
# Enable makefile generation for different targets
# ----------------------------------------------------------------------------

# Provide commit heads of current build for easier versioning
message("Build information for Fulfil.ComputerVision project:")
message(STATUS "Date Built:  ${BUILD_DATE}")
message(STATUS "Commit Head: ${DISPENSE_COMMIT}")
message(STATUS "Release Tag: ${LATEST_GIT_TAG}")
message("\nSetting up targets and build generation.")
#message("-- Current commit head for Fulfil.Dispense: ${DISPENSE_COMMIT}")

# Add the executable code for your application main function
add_executable(main main.cpp)
target_link_libraries(main Fulfil.Dispense Fulfil.DepthCam Fulfil.CPPUtils FulfilMongoCpp pthread  ${_REFLECTION} ${_GRPC_GRPCPP} ${_PROTOBUF_LIBPROTOBUF})
target_compile_definitions(main PRIVATE BUILD_DATE="${BUILD_DATE}")
target_compile_definitions(main PRIVATE FW_VERSION="${LATEST_GIT_TAG}")
target_compile_definitions(main PRIVATE DISPENSE_COMMIT="${DISPENSE_COMMIT}")


add_executable(stark_test EXCLUDE_FROM_ALL test.cpp)
target_link_libraries(stark_test Fulfil.Dispense Fulfil.DepthCam Fulfil.CPPUtils FulfilMongoCpp pthread  ${_REFLECTION} ${_GRPC_GRPCPP} ${_PROTOBUF_LIBPROTOBUF})

# testing interface that sends requests to a running API. Requires free socket connection
add_executable(dispense_test_client dispense_test_client.cpp)
target_link_libraries(dispense_test_client PRIVATE
    Fulfil.Dispense Fulfil.CPPUtils FulfilMongoCpp)

add_executable(tray_offline_simulation tray_offline_test.cpp)
target_link_libraries(tray_offline_simulation PRIVATE
    Fulfil.Dispense Fulfil.DepthCam Fulfil.CPPUtils FulfilMongoCpp)


add_executable(offline_test offline_test.cpp)
target_link_libraries(offline_test PRIVATE
    Fulfil.Dispense Fulfil.DepthCam Fulfil.CPPUtils FulfilMongoCpp)

# Remove exclusion once better targeting definitions in place
# YOU NEED TO SPECIFICALLY TARGET THESE TO TRIGGER A BUILD!
add_executable(auto_tray_calibration
   EXCLUDE_FROM_ALL
   auto_tray_calibration.cpp ../include/Fulfil.Dispense/tray/tray_process_logging_messages.h ../include/Fulfil.Dispense/recipes/visualization_presets.h)
target_link_libraries(auto_tray_calibration PRIVATE
   Fulfil.Dispense Fulfil.DepthCam Fulfil.CPPUtils FulfilMongoCpp)

add_executable(tray_calibration EXCLUDE_FROM_ALL
   tray_calibration.cpp)
target_link_libraries(tray_calibration PRIVATE Fulfil.Dispense Fulfil.DepthCam Fulfil.CPPUtils FulfilMongoCpp pthread)

# TODO Most likely should remove, unless this could be a useful foundation for a service running in parallel
find_package(CURL REQUIRED)
add_executable(upload_file EXCLUDE_FROM_ALL upload_file.cpp)
target_link_libraries(upload_file PRIVATE Fulfil.CPPUtils)

# TODO should probably include curl via Fulfil.Cpputils
add_executable(move_motor EXCLUDE_FROM_ALL move_motor.cpp)
target_link_libraries(move_motor PRIVATE Fulfil.Dispense Fulfil.DepthCam Fulfil.CPPUtils)

add_executable(camera_live_feed EXCLUDE_FROM_ALL camera_live_feed.cpp)
target_link_libraries(camera_live_feed PRIVATE Fulfil.Dispense Fulfil.DepthCam Fulfil.CPPUtils FulfilMongoCpp pthread)

add_executable(live_viewer EXCLUDE_FROM_ALL live_viewer.cpp)
target_link_libraries(live_viewer PRIVATE
        Fulfil.Dispense Fulfil.DepthCam Fulfil.CPPUtils FulfilMongoCpp)

# WARNING!!! Not tracked in Git, adjust linking as needed
# TODO REMOVE ONCE TESTING IN PLACE!!! Location to test out small functions in isolation.
option(BUILD_test_exes "SHOULD ALWAYS BE OFF! Unless you have made a scratch test exe. Some may be gitignore'd" OFF)
if(BUILD_test_exes )
    add_executable(scratch_test_private EXCLUDE_FROM_ALL
            private_scratch_test.cpp)
    target_link_libraries(scratch_test_private)

    message(WARNING "Build at your own risk! Scratch test is not tracked in Git. Be sure to add your dep here!")
    #find_package(OpenCV REQUIRED)
    #target_link_libraries(test ${OpenCV_LIBS} Fulfil.DepthCam)
endif()


