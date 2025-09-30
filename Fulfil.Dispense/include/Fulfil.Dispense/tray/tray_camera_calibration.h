#pragma once

#include <memory>
#include <iostream>
#include <filesystem>
#include <algorithm>

#include <Fulfil.Dispense/tray/tray_camera_calibration_output.h>
#include <Fulfil.DepthCam/point_cloud.h>
#include <Fulfil.DepthCam/aruco/marker.h>
#include <Fulfil.DepthCam/core.h>
#include <eigen3/Eigen/Geometry>
#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.CPPUtils/file_system_util.h>
#include <Fulfil.CPPUtils/inih/INIReader.h>
#include <Fulfil.DepthCam/aruco/marker_detector.h>
#include <json.hpp>

using fulfil::depthcam::aruco::MarkerDetector;
using fulfil::utils::FileSystemUtil;
using fulfil::utils::Logger;
using fulfil::depthcam::pointcloud::LocalPointCloud;
using fulfil::depthcam::pointcloud::PixelPointCloud;
using fulfil::depthcam::pointcloud::PointCloud;
using fulfil::depthcam::aruco::Marker;
using fulfil::dispense::tray::TrayCameraCalibrationOutput;

namespace fulfil::dispense::tray {
    class TrayCameraCalibration {
    
    public:
    
        enum CalibrationType {
            hover,
            dispense
        };
    
        // This constructor is being used in test_auto_tray_calibration.cpp for testing
        TrayCameraCalibration(std::shared_ptr<fulfil::depthcam::DeviceManager> deviceManager = nullptr);

        TrayCameraCalibration(std::shared_ptr<fulfil::depthcam::Session>& session, const std::shared_ptr<nlohmann::json>& request_json);

        TrayCameraCalibrationOutput tryCameraCalibrationCycle();
    
    private:
    
        std::string calibrationFilename(std::string_view position);
    
        template<typename BasicJsonType>
        void toJson(
            BasicJsonType& j,
            const std::shared_ptr<fulfil::depthcam::Session> session,
            const size_t& minTags
        );
    
        template<typename T>
        void addDims(const std::string& iniWritePath, T&& x, T&& y, T&& depth);
    
        void formatAddDims(
            const std::string& iniWritePath,
            const std::vector<std::pair<int, int>>& points
        );
    
        void formatAddDims(
            const std::string& iniWritePath,
            const std::vector<std::shared_ptr<Eigen::Matrix3Xd>>& points
        );
    
        void formatAddDims(
            const std::string& iniWritePath,
            const std::vector<int>& markerIds
        );
    
        void wipeFile(const std::string& iniWritePath);
    
        void writeCalibrationToIni(
            const std::string& iniWritePath,
            const std::shared_ptr<std::string> serialNumber,
            std::vector<std::pair <int, int>>& calibrationXY,
            std::vector<std::shared_ptr<Eigen::Matrix3Xd>>& cameraPoints,
            std::vector<int>& markerIds,
            CalibrationType trayPosition
        );

        void addCalibrationToOutput(
            TrayCameraCalibrationOutput& output,
            const std::shared_ptr<std::string> serialNumber,
            std::vector<std::pair <int, int>>& calibrationXY,
            std::vector<std::shared_ptr<Eigen::Matrix3Xd>>& cameraPoints,
            std::vector<int>& markerIds,
            CalibrationType trayPosition
        );
    
        bool validateMarkers(
            std::shared_ptr<fulfil::depthcam::Session> session,
            std::shared_ptr<std::vector<std::shared_ptr<Marker>>> markers,
            std::vector<std::shared_ptr<Eigen::Matrix3Xd>>& cameraPoints,
            std::vector<std::pair <int, int>>& calibrationXY,
            std::vector<int>& markerIds
        );
    
        bool getCalibration(
            std::shared_ptr<fulfil::depthcam::Session> session,
            std::vector<std::shared_ptr<Eigen::Matrix3Xd>>& cameraPoints,
            std::vector<std::pair <int, int>>& calibrationXY,
            std::vector<int>& markerIds,
            std::string position
        );
    
    
        std::array<const char*, 2> trayPositionNames{"hover", "dispense"};
        std::string iniBasePath{};
        int minMarkersRequired{16};
    
        // Calibration specific arguments
        std::shared_ptr<fulfil::depthcam::Session> session{};
        std::filesystem::path outputDir {INIReader::get_compiled_default_dir_prefix()};
        size_t minTags{12};
        CalibrationType liftHeight{hover};

        // The following are constant marker (16 aruco tags) positions on the tray (in meters) from the depth camera
        //                                  0        1        2        3        4        5        6        7        8        9        10       11       12       13       14       15
        const float markersX[16] = { -0.3180, -0.1060,  0.1060,  0.3180, -0.3180, -0.1060,  0.1060,  0.3180, -0.3180, -0.1060,  0.1060,  0.3180, -0.3180, -0.1060,  0.1060,  0.3180 };
        const float markersY[16] = { -0.3180, -0.3180, -0.3180, -0.3180, -0.1230, -0.1230, -0.1230, -0.1230,  0.0720,  0.0720,  0.0720,  0.0720,  0.2670,  0.2670,  0.2670,  0.2670 };
        const float markersZ[16] = { -0.0150, -0.0150, -0.0150, -0.0150, -0.0150, -0.0150, -0.0150, -0.0150, -0.0150, -0.0150, -0.0150, -0.0150, -0.0150, -0.0150, -0.0150, -0.0150 };
    };
    
    template<typename BasicJsonType>
    void TrayCameraCalibration::toJson(
        BasicJsonType& j,
        const std::shared_ptr<fulfil::depthcam::Session> session,
        const size_t& minTags
    ) {
        j =
        {
            { "serial_num", *session->get_serial_number() },
            { "position", trayPositionNames[liftHeight] },
            { "min_tags", minTags }
        };
    }
    
    template<typename T>
    void TrayCameraCalibration::addDims(
        const std::string& iniWritePath,
        T&& x,
        T&& y,
        T&& depth
    ) {
        INIReader::addValueToIniFile(iniWritePath,"x", std::forward<T>(x));
        INIReader::addValueToIniFile(iniWritePath,"y", std::forward<T>(y));
        INIReader::addValueToIniFile(iniWritePath,"depth", std::forward<T>(depth));
    }
}

