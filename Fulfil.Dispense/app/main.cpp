#include <memory>
#include <exception>
#include <stdexcept>

#include <Fulfil.Dispense/dispense/realsense_manager.h>
#include <Fulfil.Dispense/dispense/realsense_runner_factory.h>
#include <Fulfil.Dispense/dispense/realsense_bay_parser.h>
#include <Fulfil.Dispense/bays/bay_manager.h>

#include <Fulfil.DepthCam/core.h>
#include <Fulfil.CPPUtils/inih/ini_utils.h>

#include <Fulfil.CPPUtils/inih/INIReader.h>

#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.CPPUtils/comm/GrpcService.h>

#include <FulfilMongoCpp/mongo_connection.h>
#include <FulfilMongoCpp/mongo_objects/mongo_document.h>

#include <Fulfil.Dispense/version.h>
#include "Fulfil.Dispense/mongo/mongo_tray_calibration.h"


using ff_mongo_cpp::MongoConnection;
using ff_mongo_cpp::mongo_objects::MongoDocument;
using fulfil::utils::Logger;
using fulfil::dispense::RealsenseManager;
using fulfil::depthcam::DeviceManager;
using fulfil::dispense::RealsenseRunnerFactory;
using fulfil::dispense::RealsenseBayParser;
using fulfil::dispense::bays::BayManager;
using fulfil::depthcam::Session;
using fulfil::mongo::MongoTrayCalibration;




int main(int argc, char** argv)
{

  if (argc == 2 && (std::string(argv[1]) == "--version" || std::string(argv[1]) == "-v")) {
      // ./build/app/main [--version|-v] Returns bin version info, does not run app
    return versions::dispense_api_build_info();
  }

  // Basic logging and config set up
  std::string config_section = "device_specific";
  Logger* logger = Logger::Instance(Logger::default_logging_dir,"dispense_logs",Logger::Level::TurnOff,Logger::Level::Debug);
  auto ini_parse_log = [&logger] (std::string_view filename) {
      logger->Fatal("Failure to read and parse {} in directory {}, check path.", filename, INIReader::get_compiled_default_dir_prefix());
  };

  std::shared_ptr<INIReader> reader = std::make_shared<INIReader>("main_config.ini", true, true);
  reader->appendReader(INIReader("AGX_specific_main.ini", true));
  fulfil::utils::ini::validate_ini_parse(*reader, "AGX_specific_main.ini", ini_parse_log);

  reader->set_default_section(config_section);
  logger->SetConsoleLogLevel(reader->Get(config_section, "console_log_level", "DEBUG"));
  logger->SetFileLogLevel(reader->Get(config_section, "file_log_level", "TURN_OFF"));
  logger->Info("Latest FW Tag found: {}. Build generated on {}.\nDispense API commit details: {} ({})", FW_VERSION, BUILD_DATE, DISPENSE_COMMIT, IS_SOURCE_REPO_CLEAN);
  logger->Info("Run Fulfil.Dispense::main\n********Dispense application starting********");




  // Manager parameters

  bool both_cameras_required = reader->GetBoolean(config_section, "both_cameras_required", true);
  int expected_number_bays = reader->GetInteger(config_section, "expected_number_bays", 2); //TODO: rename to expected_number_cameras, throughout!!!
  bool frozen = reader->GetBoolean(config_section, "is_frozen", false);


  // Get mongo configs
  std::shared_ptr<INIReader> mongo_reader = std::make_shared<INIReader>("secret/mongo_conn_config.ini", true);
  fulfil::utils::ini::validate_ini_parse(*mongo_reader, "secret/mongo_conn_config.ini", ini_parse_log);
  std::string conn_str = mongo_reader->Get("connection_info", "conn_string");

  /**
   * Main control loop
   * */
  try {
    std::shared_ptr<RealsenseBayParser> parser = std::make_shared<RealsenseBayParser>(reader);

    std::shared_ptr<RealsenseManager> sensor_manager = std::make_shared<RealsenseManager>(
            std::make_shared<DeviceManager>(parser->get_camera_ids(), frozen));
    // pass in the connection here so that it can be used to retrieve configurations in dispense manager
    std::shared_ptr<RealsenseRunnerFactory> factory = std::make_shared<RealsenseRunnerFactory>(reader);
    BayManager<std::shared_ptr<Session>> manager(sensor_manager, factory, parser, expected_number_bays,
                                                 both_cameras_required, frozen);

    logger->Info("Start bay manager...");
    manager.start();

  } catch (const rs2::error & e) {
    logger->Fatal("Caught RealSense error in control loop, calling function {}({}):\n\t{}",
            e.get_failed_function(), e.get_failed_args(), e.what());
    return EXIT_FAILURE;
  } catch ( const std::exception& e ){
    logger->Fatal("Caught standard exception in last chance catch or control loop: \n\t{}", e.what());
    return EXIT_FAILURE;
  } catch(...) {
    logger->Fatal("Caught fatal non-standard exception.");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
