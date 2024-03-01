//
// Created by nkaffine on 12/11/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_DROP_TARGET_JSON_ITEM_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_DROP_TARGET_JSON_ITEM_H_
#include <memory>
#include <Fulfil.Dispense/json.hpp>

namespace fulfil
{
namespace dispense {
namespace commands
{
/**
 * The purpose of this class is to allow for easier parsing
 * of json that contains multiple items and provide a class
 * that represents how items are represented in json payloads.
 * All of the variables on this item are named such that they
 * match the key in the json that corresponds to that value.
 */
class JsonItem
{
 public:
  /**
   * JsonItem constructor
   * @param item_json a constant reference to the json that
   * contains the details of the item.
   */
  JsonItem(const nlohmann::json& item_json);

  /**
   * The length of the item in the tray in mm
   */
  double L;

  /**
   * The width of the item in the tray in mm
   */
  double W;

  /**
   * The height of the item in the tray in mm
   */
  double H;

  /**
   * Takes in json that contains an array of items and returns the
   * parsed array of json items.
   * @param item_array_json const reference to json containing an array
   * of json items details.
   * @return a pointer to a vector of json items.
   */
  static std::shared_ptr<std::vector<std::shared_ptr<fulfil::dispense::commands::JsonItem>>> parse_item_array(
      const nlohmann::json& item_array_json);
};
} // namespace commands
} // namespace dispense
} // namespace fulfil

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_DROP_TARGET_JSON_ITEM_H_
