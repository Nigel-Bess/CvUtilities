//
// Created by nkaffine on 12/12/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_TEST_FIXTURES_H_
#define FULFIL_DISPENSE_TEST_FIXTURES_H_
#include <memory>
#include <Fulfil.Dispense/json.hpp>
#include <Fulfil.Dispense/commands/drop_target/drop_target_details.h>
#include <Fulfil.Dispense/commands/drop_target/drop_target_request.h>

/**
 * The purpose of this class is to define some static
 * functions that return useful test data.
 */
class Fixtures
{
 private:
  /**
   * Gets the absolute path based on the provided filepath which
   * is relative to where the fixtures are stored.
   * @param relative_path filepath to the file relative to the
   * fixtures directory.
   * @return pointer to an absolute filepath to the file.
   */
  static std::shared_ptr<std::string> fixture_file(const char* relative_path);
  /**
   * Gets the json from the given fixture file.
   * @param relative_path path of the file relative to
   * the fixture directory.
   * @return pointer to the json.
   */
  static std::shared_ptr<nlohmann::json> get_json_from_fixture_file(const char* relative_path);
 public:


  /**
   * Returns an example pre drop request.
   * @return pointer to example pre drop request object.
   */
  static std::shared_ptr<fulfil::dispense::commands::DropTargetRequest> drop_target_request();
  /**
   * Returns an example pre drop payload
   * @return pointer to string containing data for
   * example pre drop payload.
   */
  static std::shared_ptr<std::string> drop_target_payload();
  /**
   * Returns json for an example pre drop payload.
   * @return pointer to json of pre drop payload.
   */
  static std::shared_ptr<nlohmann::json> drop_target_payload_json();
  /**
   * Returns an example pre drop details object.
   * @return pointer to example pre drop details object.
   */
  static std::shared_ptr<fulfil::dispense::commands::DropTargetDetails> drop_target_details();
  /**
   * Returns a string containing data for an example stop payload.
   * @return pointer to string containing data for an example stop payload.
   */
  static std::shared_ptr<std::string> stop_payload();
  /**
   * Returns json payload for a stop request.
   * @return pointer to json object for stop payload.
   */
  static std::shared_ptr<nlohmann::json> stop_payload_json();
  /**
   * Returns a string with data for an example id.
   * @return pointer to string containing example id.
   */
  static std::shared_ptr<std::string> example_id();
  /**
   * Returns a string containing data for a socket
   * command header
   * @param payload_size the size of the payload that will
   * be associated with the header.
   * @return pointer to string containing data for socket
   * command header.
   */
  static std::shared_ptr<std::string> header_data(int payload_size);
};

#endif //FULFIL_DISPENSE_TEST_FIXTURES_H_
