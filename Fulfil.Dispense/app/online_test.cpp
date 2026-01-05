#include <memory>
#include <exception>
#include <stdexcept>
#include <fstream>
#include <json.hpp>
#include <iostream>
#include <csignal>
#include <thread>
#include <Fulfil.OrbbecUtils/orbbec/orbbec_manager.h>
#include <Fulfil.Dispense/bays/bay_manager.h>
#include <Fulfil.DepthCam/core.h>
#include <Fulfil.DepthCam/core/sigterm.h>
#include <Fulfil.CPPUtils/inih/ini_utils.h>
#include <Fulfil.CPPUtils/inih/INIReader.h>
#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.CPPUtils/comm/GrpcService.h>
#include <Fulfil.Dispense/version.h>

// Include the SbcClient header to fix the compilation error
#include <Fulfil.CPPUtils/comm/SbcClient.h>

using fulfil::utils::Logger;
using fulfil::dispense::bays::BayManager;
using fulfil::dispense::bays::BayCameraStatusHandler;
using fulfil::depthcam::sigterm::SigTermHandler;

void run_requests(Logger* logger)
{
  logger->Info("=================================");
  logger->Info("=================================");
  logger->Info("=================================");
  logger->Info("Starting Online Test...");
  logger->Info("=================================");
  logger->Info("=================================");
  logger->Info("=================================");

  srand (time(NULL));
  int port = 9500;

  // Load "/home/fulfil/data/replay_request.json" as string
  std::string json_string = "[]";
  nlohmann::json json_objects = nlohmann::json::parse(json_string);
  try {
      std::ifstream json_file("/home/fulfil/data/replay_requests.json");
      if (json_file.is_open()) {
          json_string = std::string((std::istreambuf_iterator<char>(json_file)),
                                 std::istreambuf_iterator<char>());
          logger->Info("Successfully loaded replay requests: {}", json_string);

          // Parse as array of JSON objects
          json_objects = nlohmann::json::parse(json_string);
      } else {
          logger->Error("Failed to open /home/fulfil/data/replay_requests.json");
          return;
      }
  } catch (const std::exception& e) {
      logger->Error("Exception while loading replay request JSON string: {}", e.what());
      return;
  }

  std::string host = "0.0.0.0";
  logger->Info("Connecting to self via gRPC address {}:{}...", host, port);
  std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  auto client = std::make_shared<ClientStream>(host, port);
  std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  //logger->Info("Sending empty test payload", host, port);
  //client->Write("6949a1d36678b3d403c75ac9", "{\"Primary_Key_ID\": \"6949a1d36678b3d403c75ac9\", \"Type\": 0}");
  //auto test_response = client->ReadReponse();
  //logger->Info("Got test response!");

  // Process each JSON object in the array
  if (json_objects.is_array()) {
      for (const auto& json_object : json_objects) {
          int req_type = json_object["Type"].get<int>();
          auto pkid_val = json_object["Primary_Key_ID"].get<std::string>();
          
          // Create a mutable copy of the JSON object to modify
          auto mutable_json_object = std::make_shared<nlohmann::json>(json_object);
          /*std::string pkid_change = std::to_string(rand() % 1000 + 1);
          (*mutable_json_object)["Primary_Key_ID"] = pkid_val.substr(0, pkid_val.size() - pkid_change.size()) + pkid_change;
          logger->Info("Adding_randomness to PKID for upload test!"
                            "\n\tOld PKID: {}\n\tNew PKID: {}", pkid_val, (*mutable_json_object)["Primary_Key_ID"]);
          */
          std::this_thread::sleep_for(std::chrono::milliseconds(3000));
          client->Write(pkid_val, mutable_json_object->dump());
          logger->Info("Request Written: {}", mutable_json_object->dump());
          std::this_thread::sleep_for(std::chrono::milliseconds(3000));
          auto response = client->ReadReponse();
          logger->Info("Response was: {}", response);
      }
  } else {
      logger->Error("replay_requests.json must be an array of request objects");
  }

  logger->Info("=================================");
  logger->Info("=================================");
  logger->Info("=================================");
  logger->Info("Online Test Complete, exiting...");
  logger->Info("=================================");
  logger->Info("=================================");
  logger->Info("=================================");
}

int main(int argc, char** argv)
{

  if (argc == 2 && (std::string(argv[1]) == "--version" || std::string(argv[1]) == "-v")) {
      // ./build/app/main [--version|-v] Returns bin version info, does not run app
    return versions::dispense_api_build_info();
  }

  // Basic logging and config set up
  std::string config_section = "device_specific";
  Logger* logger = Logger::Instance(Logger::default_logging_dir, log_file_name, Logger::Level::TurnOff,Logger::Level::Debug);
  logger->SetConsoleLogLevel("TRACE");
  auto ini_parse_log = [&logger] (std::string_view filename) {
      logger->Fatal("Failure to read and parse {} in directory {}, check path.", filename, INIReader::get_compiled_default_dir_prefix());
  };

  std::shared_ptr<INIReader> reader = std::make_shared<INIReader>("main_config.ini", true, true);
  reader->appendReader(INIReader("AGX_specific_main.ini", true));
  fulfil::utils::ini::validate_ini_parse(*reader, "AGX_specific_main.ini", ini_parse_log);

  reader->set_default_section(config_section);
  logger->SetConsoleLogLevel(reader->Get(config_section, "console_log_level", "DEBUG"));
  logger->SetFileLogLevel(reader->Get(config_section, "file_log_level", "TURN_OFF"));
  // logger->Info("Latest FW Tag found: {}. Build generated on {}.\nDispense API commit details: {} ({})", FW_VERSION, BUILD_DATE, DISPENSE_COMMIT, IS_SOURCE_REPO_CLEAN);
  logger->Info("Fulfil.Dispense::main GIT info [{}]", git_info());
  logger->Info("Run Fulfil.Dispense::main\n********Dispense application starting********");

  auto bay_manager = BayManager::init_bay_manager(reader, config_section);
  logger->Info("Start bay manager...");
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  bay_manager->start(false);
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  
  run_requests(logger);

  // Trigger graceful shutdown
  std::raise(SIGTERM);
  // Wait for potential cleanup
  std::this_thread::sleep_for(std::chrono::milliseconds(3000));
}
