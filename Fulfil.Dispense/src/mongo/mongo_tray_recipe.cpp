//
// Created by amber on 10/26/20.
//

#include "Fulfil.Dispense/mongo/mongo_tray_recipe.h"
using fulfil::mongo::MongoTrayRecipe;

#include <FulfilMongoCpp/mongo_parse/mongo_bsonxx_parser.h>
using ff_mongo_cpp::mongo_parse::MongoBsonxxParser;

#include <FulfilMongoCpp/mongo_parse/mongo_bsonxx_encoder.h>
using ff_mongo_cpp::mongo_parse::MongoBsonxxEncoder;

#include "Fulfil.Dispense/tray/tray.h"
using fulfil::dispense::tray::Tray;

#include <Fulfil.CPPUtils/logging.h>
using fulfil::utils::Logger;

MongoTrayRecipe::MongoTrayRecipe(bsoncxx::document::view doc)
{
  if (doc["_id"].type() != bsoncxx::type::k_oid) this->oid = MongoObjectID();
  std::cout << "Tray had " << parse_in_values(doc) << " errors on parse.";
}

MongoTrayRecipe::MongoTrayRecipe(bsoncxx::document::value doc)
{
  if (doc.view()["_id"].type() != bsoncxx::type::k_oid) this->oid = MongoObjectID();
  std::cout << "Tray had " << parse_in_values(doc.view()) << " errors on parse.";
}

MongoTrayRecipe::MongoTrayRecipe()
{
  this->collection_name = "TrayRecipes";
  this->db_name = "Recipes";
  this->oid = MongoObjectID();
  this->lane_count = -1;
  this->tray_type_name = "Invalid";
}


void MongoTrayRecipe::SetObjectID(MongoObjectID new_id)
{
  this->oid = new_id;
}

MongoObjectID MongoTrayRecipe::GetObjectID()
{
  return MongoObjectID(this->oid);
}

std::string MongoTrayRecipe::GetCollection()
{
  return this->collection_name;
}

std::string MongoTrayRecipe::GetDataBase()
{
  return this->db_name;
}

void MongoTrayRecipe::SetCollection(const std::string& new_collection_name)
{
  this->collection_name = new_collection_name;
}

void MongoTrayRecipe::SetDataBase(const std::string& new_db_name)
{
  this->db_name = new_db_name;
}

std::shared_ptr<MongoDocument> MongoTrayRecipe::MakeNewMongoDocument(bsoncxx::document::view doc,
                                                              const std::string& collection_name, const std::string& db_name)
{
  std::shared_ptr<MongoTrayRecipe> new_doc = std::make_shared<MongoTrayRecipe>(doc);
  return new_doc;
}

void MongoTrayRecipe::update_values(bsoncxx::document::view doc){
  parse_in_values(doc);
}

int MongoTrayRecipe::parse_in_values(bsoncxx::document::view doc) {
  this->collection_name = "TrayRecipes";
  this->db_name = "Recipes";
  MongoBsonxxParser parser = MongoBsonxxParser(doc);
  this->tray_type_name = parser.getFromRoot<std::string>("name");
  this->lane_count = parser.getFromRoot<int>("LaneCount");
  // TODO should just make a float parsers bleh
  this->lane_centers = parser.getFromRoot<std::vector<double>>("LaneCenters");
  try {
    this->max_item_width = parser.getFromRoot<double>("MaxItemWidth");
    this->min_item_width = parser.getFromRoot<double>("MinItemWidth");
  } catch (std::exception& e) {
    Logger::Instance()->Warn("Unable to get Max/Min Item Width in expected format! Trying vector format.\n  Exception:\n\t{}", e.what());
    try {
      this->max_item_width = parser.getFromRoot<std::vector<double>>("MaxItemWidths")[0];
      this->min_item_width = parser.getFromRoot<std::vector<double>>("MinItemWidths")[0];
    } catch (std::exception& e) {
      this->max_item_width = 50;
      this->min_item_width = 40;
      Logger::Instance()->Error("Unable to get Max/Min Item Width in either old or new format! "
            "Using small default values min={} and max={}!\n  Exception:\n\t{}",this->min_item_width, this->max_item_width, e.what());
    }
  }
}




bsoncxx::document::value MongoTrayRecipe::MakeWritableValue() {
  MongoBsonxxEncoder tray_encoder = MongoBsonxxEncoder();
  tray_encoder.addField("name", this->tray_type_name);
  tray_encoder.addField("LaneCount", this->lane_count);
  tray_encoder.addField("LaneCenters", this->lane_centers);
  tray_encoder.addField("MaxItemWidth", this->max_item_width);
  tray_encoder.addField("MinItemWidth", this->min_item_width);
  bsoncxx::document::value final_doc = tray_encoder.extract();
  return final_doc;
}
//TODO add into inheritable class or rename
std::shared_ptr<Tray> MongoTrayRecipe::build()
{
  float min = min_item_width/1000;
  float max = max_item_width/1000;
  std::vector<float> centers;
  // TODO probably also want the dimension info to come from recipe
  for (auto c : lane_centers) centers.emplace_back(((c/1000)-0.33));
  return std::make_shared<Tray>(tray_type_name, lane_count, max, min, centers);
}

