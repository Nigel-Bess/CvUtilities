
include_directories(include/Fulfil.DepthCam/aruco)
include_directories(include/Fulfil.DepthCam/coders)
include_directories(include/Fulfil.DepthCam/core)
include_directories(include/Fulfil.DepthCam/data)
include_directories(include/Fulfil.DepthCam/frame)
include_directories(include/Fulfil.DepthCam/mocks)
include_directories(include/Fulfil.DepthCam/point_cloud)

file(GLOB_RECURSE SOURCES "*.cpp")

add_library(Fulfil.DepthCam STATIC ${SOURCES})

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
