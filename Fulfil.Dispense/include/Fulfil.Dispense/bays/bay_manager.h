//
// Created by nkaffine on 12/10/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_BAYS_BAY_MANAGER_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_BAYS_BAY_MANAGER_H_
#include <iostream>
#include <memory>
#include <thread>
#include <vector>
#include <curl/curl.h>

#include <Fulfil.CPPUtils/inih/INIReader.h>
#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.Dispense/bays/bay_runner.h>
#include <Fulfil.Dispense/bays/sensor_manager.h>
#include <Fulfil.Dispense/bays/bay_runner_factory.h>
#include <Fulfil.Dispense/bays/bay_parser.h>
#include <Fulfil.Dispense/dispense/realsense_bay_parser.h>
#include <Fulfil.DepthCam/core/depth_sensor.h>

using fulfil::depthcam::DepthSensor;
using fulfil::depthcam::DepthSession;

namespace fulfil
{
    namespace dispense {
        namespace bays
        {

class BayCameraStatusHandler : public fulfil::depthcam::DepthSensorStatusObserver {
    public:

    BayCameraStatusHandler(std::shared_ptr<DispenseBayData> bay_data);

    virtual void handle_connection_change(std::string cam_name, DepthCameras::DcCameraStatusCodes current_status) override;

    private:
    std::shared_ptr<DispenseBayData> bay_data;

};

/**
 * The purpose of this class is to handle processing requests
 * for multiple bays where each bay has a BayRunner determined
 * by the factory.
 * @tparam Sensor class to represent a sensor
 * @note this is essentially a wrapper around the threads library
 * when all threads are independent and are to run asynchronously
 * from each other but block the thread that the manager is on.
 * so this could be further abstracted if desired and put in the
 * utils library.
 * @note the implemenation of this class is in this file due to
 * the nature of building generic abstract classes in c++.
 */
            class BayManager
            {
            private:
                /**
                 * A vector of the bay runners that are running (indexed by their ids)
                 */
                std::shared_ptr<std::vector<std::shared_ptr<fulfil::dispense::bays::BayRunner>>> bays;
                /**
                 * A vector of threads that correspond the the bay runners where the index
                 * in the thread vector is the index of the bay in the bays vector.
                 */
                std::shared_ptr<std::vector<std::shared_ptr<std::thread>>> threads;
                /**
                 * The sensor manager that handles detecting the connected sensors.
                 */
                std::shared_ptr<fulfil::dispense::bays::SensorManager> manager;
                /**
                 * The factory that handles the creation of the bay runners when sensors
                 * are detected.
                 */
                std::shared_ptr<fulfil::dispense::bays::BayRunnerFactory> factory;
                /**
                 * The parser that determines which sensor goes to which factory.
                 */
                std::shared_ptr<fulfil::dispense::bays::BayParser> parser;
                std::shared_ptr<DispenseBayData> bay_data = nullptr;
            public:
                /**
                 * BayManager constructor
                 * @param sensor_manager the object that handles retrieving connected sensors to initialize
                 * the bay runners.
                 * @param runner_factory the object that handles the creation of bay runners when sensors are
                 * detected.
                 * @param sensor_parser the object that handles determining which sensor should go to which bay.
                 * @param expected_number_bays the number of bays that should be able to be created.
                 * @param both_cameras_required is a bool set in configs for whether both drop and tray cams are required for operation
                 * @note the ids that the sensor_parser returns should return an id that is not a valid index
                 * in range specified by expected_number_bays.
                 */
                BayManager(std::shared_ptr<fulfil::dispense::bays::SensorManager> sensor_manager,
                           std::shared_ptr<fulfil::dispense::bays::BayRunnerFactory> runner_factory,
                           std::shared_ptr<fulfil::dispense::bays::BayParser> sensor_parser,
                           int expected_number_bays, bool both_cameras_required, bool frozen);
                /**
                 * Kicks off each bay runner in their own thread and waits for each runner to complete before
                 * returning.
                 */
                void start();
            };

        } // namespace bays
    } // namespace dispense
} // namespace fulfil

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_BAYS_BAY_MANAGER_H_
