//
// Created by nkaffine on 12/10/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DISPENSE_REALSENSE_RUNNER_FACTORY_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DISPENSE_REALSENSE_RUNNER_FACTORY_H_
#include <Fulfil.Dispense/bays/bay_runner.h>
#include <Fulfil.Dispense/bays/bay_runner_factory.h>
#include <Fulfil.DepthCam/core.h>
#include <Fulfil.CPPUtils/inih/INIReader.h>
#include <FulfilMongoCpp/mongo_connection.h>


namespace fulfil::dispense {
/**
 * The purpose of this class is to implement the BayRunnerFactory
 * abstract class to produce bay runners that use sessions from the
 * depth cam library.
 */
class RealsenseRunnerFactory : public fulfil::dispense::bays::BayRunnerFactory<std::shared_ptr<fulfil::depthcam::Session>>
{
 private:
  /**
   * bay runner config information (mostly relating to visualization -- see main_config.ini for required info)
   * Should contain should_visualize, data_gen_image_base_dir, vid_gen_base_buffer_dir,
   */
  std::shared_ptr<INIReader>  dispense_man_reader;
  std::shared_ptr<INIReader>  tray_config_reader;
  std::shared_ptr<ff_mongo_cpp::MongoConnection> mongo_conn;

 public:
  /**
   * RealsenseRunnerFactory constructor
   * @param reader contains all the main program configs
   * @return
   */
  RealsenseRunnerFactory(std::shared_ptr<INIReader> reader, std::shared_ptr<ff_mongo_cpp::MongoConnection> conn);
  /**
   * Given the bay number and the session, creates a runner that will belong
   * to the bay with the given identifier.
   * @param bay_num the identifier for a bay.
   * @param session the session that will be attached to the bay
   * @return a runner that contains necessary aspects of the session
   * and will be owned by the bay.
   */
  std::shared_ptr<fulfil::dispense::bays::BayRunner> create(int bay_num, std::shared_ptr<fulfil::depthcam::Session> LFB_session,
                                                            std::shared_ptr<fulfil::depthcam::Session> tray_session) override;
  /**
   * Given the bay identifier, creates a bay runner without any information from the
   * session.
   * @param bay_num the identifier for the bay.
   * @return a runner that contains no details of the session.
   */
  std::shared_ptr<fulfil::dispense::bays::BayRunner> create_empty(int bay_num) override;
};
} // namespace fulfil

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DISPENSE_REALSENSE_RUNNER_FACTORY_H_
