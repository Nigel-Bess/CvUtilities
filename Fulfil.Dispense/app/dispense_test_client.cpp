//
// Created by nkaffine on 12/18/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
#include <Fulfil.CPPUtils/comm/SbcClient.h>
#include <Fulfil.CPPUtils/networking.h>
#include <Fulfil.CPPUtils/file_system_util.h>
#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.CPPUtils/inih/INIReader.h>
#include <json.hpp>
#include <FulfilMongoCpp/mongo_objects/mongo_object_id.h>

using fulfil::utils::Logger;

/**
 * The first argument should be a file with the json for the request to send. It will send the request and wait for
 * a response
 */

int main(int argc, char** argv)
{
  srand (time(NULL));
  Logger* test_logger = Logger::Instance((Logger::default_logging_dir +"/test_logs"), "dispense_test_client",
                                         Logger::Level::TurnOff,Logger::Level::Debug);
    // nlohmann::json testjson;
    // testjson["Type"] = 0;
    // testjson["Primary_Key_ID"] = "6451d1e0561da0b95c2614ad";
    // auto test = new ClientStream("0.0.0.0", 9510);
    // test->Write(testjson.dump());
    // test->ReadReponse();
  if(argc < 2)
  {
    throw std::runtime_error("Please specify which test config section to use from config ini on command line.");
  }

  std::string config_section = argv[1];

  std::unique_ptr<INIReader> reader = std::make_unique<INIReader>("test_func.ini", true);
  if (reader->ParseError() < 0) {
    test_logger->Fatal("Can't load 'test_func.ini, check path.'\n");
    throw std::runtime_error("Failure to parse ini file..");
  }

  if (reader->Sections().find(config_section) == reader->Sections().end()){
    test_logger->Fatal("The specified config section is not in test_func.ini .\n");
    throw std::invalid_argument("Test config section supplied in command line not in test ini.");
  }

  std::unique_ptr<INIReader> agx_reader = std::make_unique<INIReader>("AGX_specific_main.ini", true);

  std::string ip_addr =  agx_reader->Get("device_specific","ipAddress","Failed to find");
  int port =  agx_reader->GetInteger("device_specific","port",-1);
  std::string repo_location =  agx_reader->Get("device_specific","dispenseRepo","Failed to find");

  std::vector<std::string> json_names;
  reader->FillStringVector(config_section, "json_suffix", json_names);

    auto client = new ClientStream(ip_addr, port);
  test_logger->Info("Connection Successful. Will send a sequence of {} request(s)", json_names.size());

  // sets the amount of time between a sequence of requests will be sent. Default 2. Otherwise can input as second argument
  int time_between_requests_seconds = 2;
  if(argc == 3)
  {
    time_between_requests_seconds = atoi(argv[2]);
  }
  test_logger->Info("Will execute sequence of requests with a delay of {} seconds between them \n\t", time_between_requests_seconds);

  for (int i = 0; i < json_names.size(); i++)  //iterate through jsons and execute in order
  {
    std::string filename = repo_location;
    fulfil::utils::FileSystemUtil::join_append(filename, json_names[i]);

    test_logger->Info("Configs from ini:\n\tFilename: {}\n\tIP Address: {}\n\tPort: {}", filename, ip_addr, port);
    auto get_file_json = [](const std::string& filename){
      //std::string base_name = "/home/amber/CLionProjects/custom_json_exp/mock_request_sim/";
      std::ifstream json_file(filename);
      nlohmann::json json_obj_data;
      json_file >> json_obj_data;
      return json_obj_data;
    };
    nlohmann::json json = get_file_json(filename);
    int req_type = json["Type"].get<int>();
    if (req_type == 5 || req_type == 6){
      auto pkid_val = json["Primary_Key_ID"].get<std::string>();
      std::string pkid_change = std::to_string(rand() % 1000 + 1);
      json["Primary_Key_ID"] = pkid_val.substr(0, pkid_val.size() - pkid_change.size()) + pkid_change;
      test_logger->Info("Adding_randomness to PKID for upload test!"
                        "\n\tOld PKID: {}\n\tNew PKID: {}", pkid_val, json["Primary_Key_ID"]);
        client->Write(pkid_val, json.dump());
    }
    else client->Write("111122222233333444444555", json.dump());

    test_logger->Info("JSON REQUEST:{}", json.dump());

    // test->ReadReponse();
  }
  test_logger->Info("Ending program");
  return 0;
}