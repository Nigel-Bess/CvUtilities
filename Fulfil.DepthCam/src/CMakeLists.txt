file(GLOB DEPTHCAM_HEADER_LIST CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/include/Fulfil.DepthCam/depthcam/*.h")
file(GLOB ARUCO_HEADER_LIST CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/include/Fulfil.DepthCam/aruco/*.h")
file(GLOB MOCK_HEADER_LIST CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/include/Fulfil.DepthCam/mocks/*.h")
file(GLOB POINT_CLOUD_HEADER_LIST CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/include/Fulfil.DepthCam/point_cloud/*.h")

add_library(Fulfil.DepthCam STATIC
        ${DEPTHCAM_HEADER_LIST}
        ${ARUCO_HEADER_LIST}
        ${MOCK_HEADER_LIST}
        aruco/fixed_transform_container.cpp
        aruco/kabsch_helper.cpp
        aruco/marker.cpp
        aruco/marker_detector.cpp
        aruco/marker_detector_container.cpp
        coders/extrinsics_coder.cpp
        coders/intrinsics_coder.cpp
        coders/matrix3xd_coder.cpp
        presets/device_presets.cpp
        mocks/mock_session.cpp
        mocks/mock_session_generator.cpp
        visualization/colored_depth_display.cpp
        visualization/session_visualizer.cpp
        visualization/additive_session_visualizer.cpp
        core/depth_sensor.cpp
        core/depth_session.cpp
        core/device_manager.cpp
        core/transform_depth_session.cpp
        point_cloud/no_translation_point_cloud.cpp
        point_cloud/untranslated_point_cloud.cpp
        point_cloud/translated_point_cloud.cpp
        aruco/container_matrix3xd_predicate.cpp
        coders/point_cloud_decoder.cpp
        coders/affine3d_coder.cpp
        point_cloud/depth_pixel_point_cloud.cpp
        mocks/mock_container.cpp
        #data/FTP_generator.cpp
        data/upload_generator.cpp
        ../include/Fulfil.DepthCam/data/upload_generator.h
        ../include/Fulfil.DepthCam/data/file_sender.h
        ../include/Fulfil.DepthCam/data/null_sender.h
        #data/ftp_sender.cpp
        data/gcs_sender.cpp ../include/Fulfil.DepthCam/data/gcs_sender.h
        #data/ftp_gcs_videosender.cpp
        #data/simul_sender.cpp
        #data/basic_depth_mask_method.cpp
        #data/bigquery_upload.cpp  ../include/Fulfil.DepthCam/data/bigquery_upload.h
        frame/pixel_point_converter.cpp
        data/data_generator.cpp  ../include/Fulfil.DepthCam/data/data_generator.h
        frame/filtering.cpp
        ../include/Fulfil.DepthCam/frame/filtering.h
        ../include/Fulfil.DepthCam/data/null_sender.h)

#Adding opencv
find_package(OpenCV 4.5.5 REQUIRED)
message("-- Found OpenCV lib ${OpenCV_VERSION}!")

#Adding realsense
find_package(realsense2 REQUIRED)
message("-- Found RealSense lib ${realsense2_VERSION}!")
message("****** ${PROJECT_SOURCE_DIR}/include *****")

target_link_libraries(Fulfil.DepthCam LINK_PUBLIC json)

target_include_directories(Fulfil.DepthCam PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(Fulfil.DepthCam SYSTEM PUBLIC ${OpenCV_INCLUDE_DIRS})

#Linking opencv and realsense to the depth cam library
target_link_libraries(Fulfil.DepthCam PRIVATE ${realsense2_LIBRARY} ${OpenCV_LIBS} Fulfil.CPPUtils)
