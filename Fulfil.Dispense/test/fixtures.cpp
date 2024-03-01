//
// Created by nkaffine on 12/12/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#include "fixtures.h"
#include <Fulfil.CPPUtils/file_system_util.h>
#include <Fulfil.Dispense/commands/parsing/dispense_json_parser.h>
#include <Fulfil.CPPUtils/networking/socket_command_header.h>

using fulfil::utils::FileSystemUtil;
using fulfil::dispense::commands::DropTargetDetails;
using fulfil::dispense::commands::DispenseJsonParser;
using fulfil::utils::networking::SocketCommandHeader;
using fulfil::dispense::commands::DropTargetRequest;

std::shared_ptr<std::string> Fixtures::fixture_file(const char *relative_path)
{
  std::shared_ptr<std::string> filepath = std::make_shared<std::string>();
  filepath->append("../../test/fixtures/");
  filepath->append(relative_path);
  return filepath;
}

std::shared_ptr<nlohmann::json> Fixtures::get_json_from_fixture_file(const char *relative_path)
{
  std::shared_ptr<std::string> json_string =
      FileSystemUtil::get_string_from_file(
          fixture_file(relative_path)->c_str());
  return std::make_shared<nlohmann::json>(nlohmann::json ::parse(json_string->c_str()));
}


std::shared_ptr<std::string> Fixtures::drop_target_payload()
{
  return FileSystemUtil::get_string_from_file(
      fixture_file("pre_dispense_request.json")->c_str());
}

std::shared_ptr<nlohmann::json> Fixtures::drop_target_payload_json()
{
  return get_json_from_fixture_file("pre_dispense_request.json");
}

std::shared_ptr<fulfil::dispense::commands::DropTargetDetails> Fixtures::drop_target_details()
{
  std::shared_ptr<nlohmann::json> drop_target_json = get_json_from_fixture_file("pre_dispense_request.json");
  std::shared_ptr<DispenseJsonParser> json_parser = std::make_shared<DispenseJsonParser>();
  return std::make_shared<DropTargetDetails>(drop_target_json, std::make_shared<std::string>("000000000012"));
}

std::shared_ptr<std::string> Fixtures::stop_payload()
{
  return FileSystemUtil::get_string_from_file(fixture_file("stop_request.json")->c_str());
}

std::shared_ptr<nlohmann::json> Fixtures::stop_payload_json()
{
  return get_json_from_fixture_file("stop_request.json");
}

std::shared_ptr<std::string> Fixtures::example_id()
{
  return FileSystemUtil::get_string_from_file(fixture_file("example_id")->c_str());
}

std::shared_ptr<std::string> Fixtures::header_data(int payload_size)
{
  std::shared_ptr<std::string> header_id = FileSystemUtil::get_string_from_file(fixture_file("example_id")->c_str());
  SocketCommandHeader header;
  memcpy(&header.command_id, header_id->c_str(), 12);
  memcpy(&header.bytesleft, &payload_size, 2);
  char* header_data = new char[14];
  memcpy(header_data, &header, 14);
  return std::make_shared<std::string>(header_data, sizeof(SocketCommandHeader));
}

std::shared_ptr<DropTargetRequest> Fixtures::drop_target_request()
{
  return std::make_shared<DropTargetRequest>(example_id(), example_id(), drop_target_details(),std::make_shared<nlohmann::json>());
}
