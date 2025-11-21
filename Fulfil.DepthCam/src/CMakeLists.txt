# Fulfil.DepthCam/src/CMakeLists.txt

# Public include root for DepthCam
set(FULFIL_DEPTHCAM_PUBLIC_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")

# Subdirectory includes for DepthCam
include_directories("${FULFIL_DEPTHCAM_PUBLIC_INCLUDE_DIR}/Fulfil.DepthCam/aruco")
include_directories("${FULFIL_DEPTHCAM_PUBLIC_INCLUDE_DIR}/Fulfil.DepthCam/coders")
include_directories("${FULFIL_DEPTHCAM_PUBLIC_INCLUDE_DIR}/Fulfil.DepthCam/core")
include_directories("${FULFIL_DEPTHCAM_PUBLIC_INCLUDE_DIR}/Fulfil.DepthCam/data")
include_directories("${FULFIL_DEPTHCAM_PUBLIC_INCLUDE_DIR}/Fulfil.DepthCam/frame")
include_directories("${FULFIL_DEPTHCAM_PUBLIC_INCLUDE_DIR}/Fulfil.DepthCam/mocks")
include_directories("${FULFIL_DEPTHCAM_PUBLIC_INCLUDE_DIR}/Fulfil.DepthCam/point_cloud")

file(GLOB_RECURSE FULFIL_DEPTHCAM_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
)

add_library(Fulfil.DepthCam STATIC ${FULFIL_DEPTHCAM_SOURCES})

set(_fulfil_depthcam_lib_dir "${CMAKE_BINARY_DIR}/lib")
set(_fulfil_depthcam_bin_dir "${CMAKE_BINARY_DIR}/bin")

set_target_properties(Fulfil.DepthCam PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${_fulfil_depthcam_lib_dir}"
    LIBRARY_OUTPUT_DIRECTORY "${_fulfil_depthcam_lib_dir}"
    RUNTIME_OUTPUT_DIRECTORY "${_fulfil_depthcam_bin_dir}"
    OUTPUT_NAME "Fulfil.DepthCam"
)

#Adding opencv
find_package(OpenCV 4.5.5 REQUIRED)
message("-- Found OpenCV lib ${OpenCV_VERSION}!")

#Adding realsense
find_package(realsense2 REQUIRED)
message("-- Found RealSense lib ${realsense2_VERSION}!")
message("****** ${PROJECT_SOURCE_DIR}/include *****")

target_link_libraries(Fulfil.DepthCam LINK_PUBLIC json)

# Public include dirs for users of DepthCam
target_include_directories(Fulfil.DepthCam
    PUBLIC
        "${FULFIL_DEPTHCAM_PUBLIC_INCLUDE_DIR}"
    SYSTEM PUBLIC
        ${OpenCV_INCLUDE_DIRS}
    PRIVATE
        "${PROJECT_SOURCE_DIR}/third-party/json/include"
)

# Link OpenCV, RealSense, and the prebuilt Fulfil.CPPUtils + Fulfil.OrbbecUtils
target_link_libraries(Fulfil.DepthCam
    PUBLIC
        ${realsense2_LIBRARY}
        ${OpenCV_LIBS}
        Fulfil.CPPUtils
        Fulfil.OrbbecUtils
)
