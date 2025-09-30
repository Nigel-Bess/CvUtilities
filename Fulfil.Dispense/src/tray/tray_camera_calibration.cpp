#include <Fulfil.Dispense/tray/tray_camera_calibration.h>

using fulfil::dispense::tray::TrayCameraCalibration;

TrayCameraCalibration::TrayCameraCalibration(
    std::shared_ptr<fulfil::depthcam::DeviceManager> deviceManager
) {
    if (deviceManager == nullptr)
        deviceManager = std::make_shared<fulfil::depthcam::DeviceManager>();

    this->session = deviceManager->get_connected_sessions()->back();
    Logger::Instance()->Warn("Using default serial {}!", *this->session->get_serial_number());
    this->session->set_sensor_name("Calibration_DAB");
}

TrayCameraCalibration::TrayCameraCalibration(
    std::shared_ptr<fulfil::depthcam::Session>& session,
    const std::shared_ptr<nlohmann::json>& request_json
) {

    this->session = session;
    Logger::Instance()->Info("Using camera with serial {} !", *this->session->get_serial_number());

    this->liftHeight = (*request_json)["position"] == "hover" ? TrayCameraCalibration::hover : TrayCameraCalibration::dispense;
    Logger::Instance()->Info("Height (lift position): {}", trayPositionNames[this->liftHeight]);
}

std::string TrayCameraCalibration::calibrationFilename(std::string_view position) {
    return std::string("tray_calibration_data_").append(*session->get_serial_number()).append("_").append(position) + ".ini";
}

void TrayCameraCalibration::formatAddDims(
    const std::string& iniWritePath,
    const std::vector<std::pair<int, int>>& points
) {
    std::vector<int> x;
    std::vector<int> y;
    std::vector<int> depth;
    auto unzipPoint = [&x, &y, &depth](std::pair <int, int> pixel) {x.push_back(pixel.first); y.push_back(pixel.second); depth.push_back(0); };
    std::for_each( points.begin(), points.end(), unzipPoint);
    addDims(iniWritePath, x, y, depth);
}

void TrayCameraCalibration::formatAddDims(
    const std::string& iniWritePath,
    const std::vector<std::shared_ptr<Eigen::Matrix3Xd>>& points
) {
    std::vector<double> x;
    std::vector<double> y;
    std::vector<double> depth;
    auto unzipPoint = [&x, &y, &depth](std::shared_ptr<Eigen::Matrix3Xd> point) {
        x.push_back(((*point)(0,0)));
        y.push_back(((*point)(1,0)));
        depth.push_back(((*point)(2,0)));
    };
    std::for_each( points.begin(), points.end(), unzipPoint);
    addDims(iniWritePath, x, y, depth);
}

void TrayCameraCalibration::formatAddDims(
    const std::string& iniWritePath,
    const std::vector<int>& markerIds
) {
    std::string x = "";
    std::string y = "";
    std::string d = "";
    for (int i = 0; i < (int)markerIds.size(); i++) {
        x = x + std::to_string(markersX[markerIds.at(i)]) + " ";
        y = y + std::to_string(markersY[markerIds.at(i)]) + " ";
        d = d + std::to_string(markersZ[markerIds.at(i)]) + " ";
    }
    addDims(iniWritePath, x, y, d);
}

void TrayCameraCalibration::wipeFile(const std::string& iniWritePath) {

    std::ofstream del;
    del.open(iniWritePath, std::ofstream::out | std::ofstream::trunc);
    del.close();
}

void TrayCameraCalibration::addCalibrationToOutput(
    TrayCameraCalibrationOutput& output,
    const std::shared_ptr<std::string> serialNumber,
    std::vector<std::pair <int, int>>& calibrationXY,
    std::vector<std::shared_ptr<Eigen::Matrix3Xd>>& cameraPoints,
    std::vector<int>& markerIds,
    CalibrationType trayPosition
) {

    if (!serialNumber) {
        throw std::runtime_error("addCalibrationToOutput: Serial number is null - cannot add calibration to output");
    }

    try {
        char hostname[255];
        memset(hostname, 0, sizeof(hostname));
        if (gethostname(hostname, sizeof(hostname)) != 0) {
            throw std::runtime_error("addCalibrationToOutput: Failed to get machine name");
        }

        // Set machine name
        output.machine_name = std::string(hostname);

        // Set camera serial number
        output.camera_serial = *serialNumber;
        
        // Set position based on tray position
        output.position = (trayPosition == CalibrationType::hover) ? "hover" : "dispense";
        
        // Set timestamp
        output.updated_at = std::chrono::system_clock::now();
        
        // Fill pixel coordinates
        for (size_t i = 0; i < calibrationXY.size() && i < TrayCameraCalibrationOutput::num_markers; ++i) {
            output.pixel_coordinates.points[i].x = static_cast<float>(calibrationXY[i].first);
            output.pixel_coordinates.points[i].y = static_cast<float>(calibrationXY[i].second);
            output.pixel_coordinates.points[i].z = 0.0f; // Pixel coordinates don't have Z
        }
        
        // Fill camera depth coordinates
        for (size_t i = 0; i < cameraPoints.size() && i < TrayCameraCalibrationOutput::num_markers; ++i) {
            if (cameraPoints[i] && cameraPoints[i]->cols() > 0) {
                // Assuming the Eigen::Matrix3Xd contains 3D points where each column is a point
                output.camera_depth_coordinates.points[i].x = static_cast<float>((*cameraPoints[i])(0, 0));
                output.camera_depth_coordinates.points[i].y = static_cast<float>((*cameraPoints[i])(1, 0));
                output.camera_depth_coordinates.points[i].z = static_cast<float>((*cameraPoints[i])(2, 0));
            }
        }
        
        // Fill tray coordinates
        for (size_t i = 0; i < markerIds.size() && i < TrayCameraCalibrationOutput::num_markers; ++i) {
            output.tray_depth_coordinates.points[i].x = static_cast<float>(markersX[i]);
            output.tray_depth_coordinates.points[i].y = static_cast<float>(markersY[i]);
            output.tray_depth_coordinates.points[i].z = static_cast<float>(markersZ[i]);
        }
        
        // Set success status
        output.return_code = DcApiErrorCode::Success;
        output.error_description = "";
        
    } catch (const std::exception& e) {
        throw std::runtime_error("addCalibrationToOutput: Failed to add calibration to output - " + std::string(e.what()));
    }
}

void TrayCameraCalibration::writeCalibrationToIni(
    const std::string& iniWritePath,
    const std::shared_ptr<std::string> serialNumber,
    std::vector<std::pair <int, int>>& calibrationXY,
    std::vector<std::shared_ptr<Eigen::Matrix3Xd>>& cameraPoints,
    std::vector<int>& markerIds,
    CalibrationType trayPosition
) {

    if(trayPosition == CalibrationType::dispense)
    {
        INIReader::addSectionToIniFile(iniWritePath, *serialNumber + std::string("_pixel_locations"));
        formatAddDims(iniWritePath, calibrationXY);
        INIReader::addSectionToIniFile(iniWritePath, *serialNumber + std::string("_camera_coordinates"));
        formatAddDims(iniWritePath, cameraPoints);
        INIReader::addSectionToIniFile(iniWritePath, *serialNumber + std::string("_tray_coordinates"));
        formatAddDims(iniWritePath, markerIds);
    }
    else if(trayPosition == CalibrationType::hover)
    {
        INIReader::addSectionToIniFile(iniWritePath, *serialNumber + std::string("_hover_pixel_locations"));
        formatAddDims(iniWritePath, calibrationXY);
        INIReader::addSectionToIniFile(iniWritePath, *serialNumber + std::string("_hover_camera_coordinates"));
        formatAddDims(iniWritePath, cameraPoints);
        INIReader::addSectionToIniFile(iniWritePath, *serialNumber + std::string("_hover_tray_coordinates"));
        formatAddDims(iniWritePath, markerIds);
    }
}

bool TrayCameraCalibration::validateMarkers(
    std::shared_ptr<fulfil::depthcam::Session> session,
    std::shared_ptr<std::vector<std::shared_ptr<Marker>>> markers,
    std::vector<std::shared_ptr<Eigen::Matrix3Xd>>& cameraPoints,
    std::vector<std::pair <int, int>>& calibrationXY,
    std::vector<int>& markerIds
) {

    if (!session) {
        throw std::runtime_error("validateMarkers: Session is null - cannot validate markers");
    }

    if (!markers) {
        throw std::runtime_error("validateMarkers: Markers vector is null - cannot validate markers");
    }

    //check that markers are within search zone and at expected depth
    int numDetectedMarkers = markers->size();

    std::stringstream msgs;
    msgs << "Detected " << numDetectedMarkers << " markers with IDs: ";
    for (int i = 0; i < numDetectedMarkers; i++)
    {
        std::shared_ptr<Marker> marker = markers->at(i);
        msgs << marker->get_id() << ' ';
    }
    msgs << '\n';

    if(numDetectedMarkers < minMarkersRequired)
    {
        Logger::Instance()->Error("Did not detect at least {} markers as expected. {}", minMarkersRequired, msgs.str());
        throw std::runtime_error("validateMarkers: Insufficient markers detected - expected at least " + std::to_string(minMarkersRequired) + " but found " + std::to_string(numDetectedMarkers));
    }
    Logger::Instance()->Info("Found min number of markers {}. {}", minMarkersRequired, msgs.str());

    float depthMin = 0.5;
    float depthMax = 1.5;

    for(int i = 0; i < numDetectedMarkers; i++)
    {
        std::shared_ptr<Marker> marker = markers->at(i);
        if (!marker) {
            throw std::runtime_error("validateMarkers: Marker at index " + std::to_string(i) + " is null");
        }

        int markerId = marker->get_id();
        float markerX = marker->get_coordinate(fulfil::depthcam::aruco::Marker::center)->x;
        float markerY = marker->get_coordinate(fulfil::depthcam::aruco::Marker::center)->y;

        //validate marker is within acceptable region of RGB image. Calls depth session function to use aligned depth frame
        float detectedDepth = session->depth_at_pixel(round(markerX), round(markerY));
        Logger::Instance()->Debug("Validating location and depth of marker: {}", markerId);

        if(detectedDepth < depthMin or detectedDepth > depthMax)
        {
            Logger::Instance()->Warn("Marker ID {} at ({},{})  had detected depth outside of acceptable range and was removed. Detected: {}, min acceptable: {}, max acceptable: {}",
                                     marker->get_id(), markerX, markerY, detectedDepth, depthMin, depthMax);
            throw std::runtime_error("validateMarkers: Marker ID " + std::to_string(markerId) + " has invalid depth: " + std::to_string(detectedDepth) + " (expected between " + std::to_string(depthMin) + " and " + std::to_string(depthMax) + ")");
        }
        
        try {
            //get camera coordinates, this is required for the calibration file
            std::shared_ptr<std::vector<std::shared_ptr<std::pair<std::shared_ptr<cv::Point2f>, float>>>> point =
                    std::make_shared<std::vector<std::shared_ptr<std::pair<std::shared_ptr<cv::Point2f>, float>>>>();
            point->push_back(std::make_shared<std::pair<std::shared_ptr<cv::Point2f>,float>>(std::make_shared<cv::Point2f>(markerX, markerY), detectedDepth));
            std::shared_ptr<PointCloud> cloud = session->get_point_cloud(true, __FUNCTION__)->as_pixel_cloud()->new_point_cloud(point);
            if (!cloud) {
                throw std::runtime_error("validateMarkers: Failed to create point cloud for marker " + std::to_string(markerId));
            }
            std::shared_ptr<Eigen::Matrix3Xd> data = cloud->as_camera_cloud()->get_data();
            if (!data) {
                throw std::runtime_error("validateMarkers: Failed to get camera data for marker " + std::to_string(markerId));
            }

            calibrationXY.push_back(std::pair <int, int>(markerX, markerY));
            cameraPoints.push_back(data);
            markerIds.push_back(markers->at(i)->get_id());
        } catch (const std::exception& e) {
            throw std::runtime_error("validateMarkers: Failed to process camera coordinates for marker " + std::to_string(markerId) + ": " + e.what());
        }
    }
    return true;
}

bool TrayCameraCalibration::getCalibration(
    std::shared_ptr<fulfil::depthcam::Session> session,
    std::vector<std::shared_ptr<Eigen::Matrix3Xd>>& cameraPoints,
    std::vector<std::pair <int, int>>& calibrationXY,
    std::vector<int>& markerIds,
    std::string position
) {
    if (!session) {
        throw std::runtime_error("getCalibration: Session is null - cannot perform calibration");
    }

    try {
        std::string writePath = iniBasePath + "/" + position + "_calibration_image.jpg";
        Logger::Instance()->Info("Saving image to {}", writePath);
        session->refresh();
        
        auto colorMat = session->get_color_mat();
        if (!colorMat) {
            throw std::runtime_error("getCalibration: Failed to get color image from session");
        }
        
        bool writeSuccess = cv::imwrite(writePath, *colorMat);
        if (!writeSuccess) {
            throw std::runtime_error("getCalibration: Failed to write calibration image to: " + writePath);
        }

        Logger::Instance()->Info("Captured and wrote image to local device. Running marker detector now.");
        MarkerDetector detector = MarkerDetector(16, 5);

        // 10 attempts at finding and validating markers
        int numAttempts = 10;
        for(int i = 0; i < numAttempts; i++)
        {
            Logger::Instance()->Info("Marker Detector attempt: {}", i);
            
            try {
                std::shared_ptr<std::vector<std::shared_ptr<Marker>>> detectedMarkers = detector.detect_markers(session->get_color_mat());
                if (!detectedMarkers) {
                    throw std::runtime_error("getCalibration: Marker detector returned null markers vector");
                }
                
                bool validationCheck = validateMarkers(session, detectedMarkers, cameraPoints, calibrationXY, markerIds); //validates depth and within search region
                if(validationCheck) break; //break out of loop if all markers were detected and valid
            } catch (const std::exception& e) {
                Logger::Instance()->Warn("Marker detection/validation failed on attempt {}: {}", i, e.what());
                if(i == numAttempts - 1) {
                    throw std::runtime_error("getCalibration: Failed to validate markers after maximum attempts. Last error: " + std::string(e.what()));
                }
            }
            
            session->refresh(); //take a new image to try again
        }

        Logger::Instance()->Info("Markers all found to be valid, and camera coordinates for calibration have been found.");
        return true;
    } catch (const std::exception& e) {
        throw std::runtime_error("getCalibration: Calibration failed - " + std::string(e.what()));
    }
}

TrayCameraCalibrationOutput TrayCameraCalibration::tryCameraCalibrationCycle() {

    TrayCameraCalibrationOutput output;
    
    try {
        if (!session) {
            throw std::runtime_error("tryCameraCalibrationCycle: Session is null - cannot perform calibration cycle");
        }

        std::string iniPath = outputDir / calibrationFilename(trayPositionNames[this->liftHeight]);
        Logger::Instance()->Info("Wiping file: {}", iniPath);
        wipeFile(iniPath);

        std::shared_ptr<std::string> serialNumber = session->get_serial_number();
        if (!serialNumber) {
            throw std::runtime_error("tryCameraCalibrationCycle: Failed to get camera serial number");
        }
        
        std::cout << "We will begin calibrating camera serial no. " << *serialNumber << std::endl;

        std::vector<std::shared_ptr<Eigen::Matrix3Xd>> cameraPoints;
        std::vector<std::pair <int, int>> calibrationXY;
        std::vector<int> markerIds;

        std::string position;
        switch (liftHeight) {
            case CalibrationType::dispense:
                position = "dispense";
                break;
            case CalibrationType::hover:
                position = "hover";
                break;
            default:
                throw std::runtime_error("tryCameraCalibrationCycle: Invalid calibration type: " + std::to_string(static_cast<int>(liftHeight)));
        }
        std::cout << "position " << position << std::endl;

        // Perform calibration
        bool success = getCalibration(session, cameraPoints, calibrationXY, markerIds, position);
        
        if (!success) {
            throw std::runtime_error("tryCameraCalibrationCycle: Calibration failed - getCalibration returned false");
        }

        Logger::Instance()->Info("Writing new calibration position to file now.");

        try {
            char hostname[255];
            memset(hostname, 0, sizeof(hostname));
            if (gethostname(hostname, sizeof(hostname)) != 0) {
                Logger::Instance()->Warn("Failed to get hostname for INI file");
            }

            INIReader::addSectionToIniFile(iniPath, "config_details", false);
            INIReader::addValueToIniFile(iniPath,"hostname", std::string(hostname));
            time_t now = std::time(0);
            INIReader::addValueToIniFile(iniPath,"date_updated", now);

            writeCalibrationToIni(iniPath, serialNumber, calibrationXY, cameraPoints, markerIds, liftHeight);
        } catch (const std::exception& e) {
            Logger::Instance()->Warn("Failed to write INI file, but continuing with struct population: {}", e.what());
        }

        Logger::Instance()->Info("Adding new calibration position to TrayCameraCalibrationOutput struct !");
        addCalibrationToOutput(output, serialNumber, calibrationXY, cameraPoints, markerIds, liftHeight);

        Logger::Instance()->Info("Calibration was a success!!!");
        return output;
        
    } catch (const std::runtime_error& e) {
        Logger::Instance()->Error("Runtime error during calibration: {}", e.what());
        output.return_code = DcApiErrorCode::UnspecifiedError;
        output.error_description = std::string("Runtime error: ") + e.what();
        return output;
    } catch (const std::exception& e) {
        Logger::Instance()->Error("Exception during calibration: {}", e.what());
        output.return_code = DcApiErrorCode::UnspecifiedError;
        output.error_description = std::string("Exception: ") + e.what();
        return output;
    } catch (...) {
        Logger::Instance()->Error("Unknown exception during calibration");
        output.return_code = DcApiErrorCode::UnspecifiedError;
        output.error_description = "Unknown exception occurred during calibration";
        return output;
    }
}
