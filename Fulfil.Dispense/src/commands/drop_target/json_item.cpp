//
// Created by nkaffine on 12/11/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#include "Fulfil.Dispense/commands/drop_target/json_item.h"

using fulfil::dispense::commands::JsonItem;

JsonItem::JsonItem(const nlohmann::json &item_json)
{
  this->L = item_json["L"].get<float>();
  this->W = item_json["W"].get<float>();
  this->H = item_json["H"].get<float>();
}

std::shared_ptr<std::vector<std::shared_ptr<JsonItem>>> JsonItem::parse_item_array(const nlohmann::json &item_array_json)
{
  // Creating an empty list of json items
  std::shared_ptr<std::vector<std::shared_ptr<JsonItem>>> items = std::make_shared<std::vector<std::shared_ptr<JsonItem>>>();
  //Iterating over the items in the json list.
  for (auto item_json = item_array_json.begin(); item_json != item_array_json.end(); ++item_json)
  {
    // Adding the item to the list of json items.
    std::shared_ptr<JsonItem> item = std::make_shared<JsonItem>(item_json.value());
    items->push_back(item);
  }
  return items;
}
