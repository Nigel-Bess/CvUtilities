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

namespace fulfil
{
    namespace dispense {
        namespace bays
        {
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
            template <class Sensor>
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
                std::shared_ptr<fulfil::dispense::bays::SensorManager<Sensor>> manager;
                /**
                 * The factory that handles the creation of the bay runners when sensors
                 * are detected.
                 */
                std::shared_ptr<fulfil::dispense::bays::BayRunnerFactory<Sensor>> factory;
                /**
                 * The parser that determines which sensor goes to which factory.
                 */
                std::shared_ptr<fulfil::dispense::bays::BayParser<Sensor>> parser;
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
                BayManager(std::shared_ptr<fulfil::dispense::bays::SensorManager<Sensor>> sensor_manager,
                           std::shared_ptr<fulfil::dispense::bays::BayRunnerFactory<Sensor>> runner_factory,
                           std::shared_ptr<fulfil::dispense::bays::BayParser<Sensor>> sensor_parser,
                           int expected_number_bays, bool both_cameras_required, bool frozen);
                /**
                 * Kicks off each bay runner in their own thread and waits for each runner to complete before
                 * returning.
                 */
                void start();
            };

            template <class Sensor>
            fulfil::dispense::bays::BayManager<Sensor>::BayManager(std::shared_ptr<fulfil::dispense::bays::SensorManager<Sensor>> sensor_manager,
                                                                   std::shared_ptr<fulfil::dispense::bays::BayRunnerFactory<Sensor>> runner_factory,
                                                                   std::shared_ptr<fulfil::dispense::bays::BayParser<Sensor>> sensor_parser,
                                                                   int expected_number_bays, bool both_cameras_required, bool frozen) :
            manager{sensor_manager} , factory{runner_factory}, parser{sensor_parser}
            {
              auto expected_bays = parser->get_bay_ids();
              this->bays = std::make_shared<std::vector<std::shared_ptr<BayRunner>>>(); //TODO: This can be refactored now that we just have one bay
              try
              {
                std::shared_ptr<std::vector<std::shared_ptr<fulfil::depthcam::Session>>> sensors =
                    manager->get_connected_sensors();
                int num_sensors = sensors->size();


                if (num_sensors == 0) {
                  fulfil::utils::Logger::Instance()->Error("No Cameras Found;");
                  // TODO informative throw
                  exit(4); //Exit program is no cameras are found to be connected
                }


                for (auto config_bay_id : parser->get_bay_ids()) {
                  std::vector<std::shared_ptr<fulfil::depthcam::Session>> bay_sensors =
                    parser->get_bay(sensors, config_bay_id);
                  auto bay_data = DispenseBayData{ bay_sensors, config_bay_id, both_cameras_required };
                  // int sensor_num_1 = parser->get_bay(sensors, 1);
                  if (bay_data.can_init())// if one of the first bay's sensors was found
                  {
                      fulfil::utils::Logger::Instance()->Info(
                        "Initializing Bay {} because start condition was met!", bay_data.bay_id);

                      // This will check for valid frame data
                      int frame_check_iterations = 10;
                      auto log_refresh = [&iters=frame_check_iterations](auto session, auto tag) {
                          if (session) {
                              fulfil::utils::Logger::Instance()->Info("Initializing {} cameras frame check with {} iterations ", tag, iters);
                              for (int i = 0; i < iters; i++) {
                                  if (i % 2 == 0) { fulfil::utils::Logger::Instance()->Info("Currently at {} iteration: {}", tag, i);}
                                  //session->refresh();
                      }}};
                      log_refresh(bay_data.lfb_session, "LFB");
                      log_refresh(bay_data.tray_session, "Tray");
                      std::shared_ptr<BayRunner> runner = factory->create(
                        std::stoi(bay_data.bay_id), bay_data.lfb_session, bay_data.tray_session);
                      bays->push_back(runner->get_shared_ptr());
                      runner->bind_delegates();
                      fulfil::utils::Logger::Instance()->Debug("Initialized thread for bay {}", bay_data.bay_id);
                  } else {
                      fulfil::utils::Logger::Instance()->Error(
                        "Starting condition not met for bay!; Found {} sensors required by bay {}!",
                        bay_data.num_valid_session(),
                        bay_data.bay_id);
                  }
                }
              }
              catch (const std::exception &e)
              {
                fulfil::utils::Logger::Instance()->Error("Issue building Sensors, Bays or Runners; Vars: error = {}", e.what());
                exit(6);
              }
              if (this->bays->empty()) {
                fulfil::utils::Logger::Instance()->Error("Failed to initialize any bays out of the {} requested!", expected_bays.size());
                exit(6);
              }
              this->threads = std::make_shared<std::vector<std::shared_ptr<std::thread>>>(bays->size()); //TODO: reduce number of threads here?

            }

            template <class Sensor>
            void fulfil::dispense::bays::BayManager<Sensor>::start()
            {
              curl_global_init(CURL_GLOBAL_ALL);
              for(int i = 0; i < this->bays->size(); i++)
              {
                if (this->bays->at(i) == nullptr) continue; //Do not try to start bays that have not been created
                threads->at(i) = std::make_shared<std::thread>([this, i]() {
                    this->bays->at(i)->start();
                });
              }
              for(int i = 0; i < this->threads->size(); i++)
              {
                if (this->bays->at(i) == nullptr) continue;
                this->threads->at(i)->join();
              }
            }


        } // namespace bays
    } // namespace dispense
} // namespace fulfil

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_BAYS_BAY_MANAGER_H_
