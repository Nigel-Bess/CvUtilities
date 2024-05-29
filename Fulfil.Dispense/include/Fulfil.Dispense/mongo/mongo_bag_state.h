//
// Created by steve on 1/21/21.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_MONGO_BAG_STATE_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_MONGO_BAG_STATE_H_

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>

#include <FulfilMongoCpp/mongo_objects/mongo_document.h>
#include <FulfilMongoCpp/mongo_objects/mongo_object_id.h>
#include <FulfilMongoCpp/mongo_objects/mongo_element.h>

#include<opencv2/opencv.hpp>
#include <Fulfil.CPPUtils/eigen.h>
#include <Fulfil.Dispense/mongo/packing_state.h>
#include "Fulfil.Dispense/recipes/lfb_vision_configuration.h"

using ff_mongo_cpp::mongo_objects::MongoObjectID;
using fulfil::configuration::lfb::LfbVisionConfiguration;

namespace fulfil::mongo
{
class CvBagState final
{
    public:
        CvBagState(nlohmann::json json)
        {
            _bag_id_string = json["BagId"].get<std::string>();
            // for offline test compatibility - the key is MongoID in prod but _id in offline data
            _id_string = json.value("MongoID", "NOT FOUND");
            bool is_offline_test_run = false;
            if (_id_string == "NOT FOUND") {
                is_offline_test_run = true;
                _id_string = json["_id"].get<std::string>();
            }

            BagId = MongoObjectID(bsoncxx::oid(_bag_id_string));
            MongoID = MongoObjectID(bsoncxx::oid(_id_string));
            std::cout << "item map 1 json " << json["ItemMap1"] << std::endl;

            ItemMap1 = json["ItemMap1"].get<std::vector<int>>();

            ItemMap2 = json["ItemMap2"].get<std::vector<int>>();

            ItemMap3 = json["ItemMap3"].get<std::vector<int>>();

            PackedItemsVolume = json["PackedItemsVolume"].get<int>();
            PercentBagFull = json["PercentBagFull"].get<int>();
            PackingEfficiency = json["PackingEfficiency"].get<int>();
            NumberDamageRejections = json["NumberDamageRejections"].get<int>();
            if (is_offline_test_run) {
                Config = std::make_shared<LfbVisionConfiguration>();
            } else {
                Config = std::make_shared<LfbVisionConfiguration>(std::make_shared<nlohmann::json>(json["LfbConfig"]));
            }
            std::cout << "CvBagState updated: LFB Gen is " << Config->lfb_generation << std::endl;
        }
        CvBagState(){  }
        ff_mongo_cpp::mongo_objects::MongoObjectID BagId;
        std::vector<int> ItemMap1;
        std::vector<int> ItemMap2;
        std::vector<int> ItemMap3;
        int PackedItemsVolume = -1;
        int PercentBagFull = -1;
        int PackingEfficiency = -1;
        int NumberDamageRejections= -1;
        ff_mongo_cpp::mongo_objects::MongoObjectID MongoID;
        std::shared_ptr<LfbVisionConfiguration> Config;

        nlohmann::json ToJson()
        {
            nlohmann::json json;
            json["BagId"] = _bag_id_string;
            json["_id"] = _id_string;
            json["ItemMap1"] = ItemMap1;
            json["ItemMap2"] = ItemMap2;
            json["ItemMap3"] = ItemMap3;
            json["PackedItemsVolume"] = PackedItemsVolume;
            json["PercentBagFull"] = PercentBagFull;
            json["PackingEfficiency"] = PackingEfficiency;
            json["NumberDamageRejections"] = NumberDamageRejections;
//            if (include_lfb_vision_config) {
//                nlohmann::json config_json;
//                config_json
//                json["LfbConfig"] = config_json;
//            }
            return json;
        }
        std::string ToString(){
            return nlohmann::to_string(this->ToJson());
        }

        
    private:
        std::string _bag_id_string;
        std::string _id_string;
};

class MongoBagState : public ff_mongo_cpp::mongo_objects::MongoDocument
{
 public:
  MongoBagState();
  MongoBagState(ff_mongo_cpp::mongo_objects::MongoObjectID bag_id, bool new_doc);

  ~MongoBagState() = default;

  void CreateDefaultMap();

  std::shared_ptr<CvBagState> raw_mongo_doc;

  ff_mongo_cpp::mongo_objects::MongoObjectID GetObjectID();
  std::string GetCollection();
  std::string GetDataBase();
  void SetCollection(const std::string& collection_name);
  void SetDataBase(const std::string& db_name);

  void SetObjectID(ff_mongo_cpp::mongo_objects::MongoObjectID new_id);

  inline void update_values(bsoncxx::document::view doc) {}
  /**
   *  Takes in a pointer to a vector of cv::Mat, where each Mat is one layer of the item map,
   *  and converts this into item_map_arrays for mongo.
   */
  void set_item_map_arrays_from_mat(std::shared_ptr<std::vector<cv::Mat>> item_map_ptr);

  bsoncxx::document::value MakeWritableValue();
  void UpdateRawMongoDocState();
  nlohmann::json GetStateAsJson();
  std::string GetStateAsString();

  std::shared_ptr<ff_mongo_cpp::mongo_objects::MongoDocument> MakeNewMongoDocument(bsoncxx::document::view doc,
                                                      const std::string& collection_name = "",
                                                      const std::string& db_name = "");

  /**
   *  Converts the arrays representation of the item map into a cv::Mat that can be easily displayed
   */
  std::vector<cv::Mat> build_cv_item_map();


  /**
   *  Takes in a pointer to a vector of cv::Mat, which represents the state of new item(s) dispensed into the bag.
   *  The function uses this map to update the item_map_arrays. New item status is added to the first layer, and
   *  if items already exist on that layer, they are moved in the item map to deeper layers
   *  If the comparison_result == nullptr, that means there were no new items detected in the bag. Update not required
   */
  void update_item_map(std::shared_ptr<cv::Mat> comparison_result, int material_type);

  /**
   *  Get a cv::Mat of all zeros for visualization purposes, with correct num_rows and num_cols
   */
  std::shared_ptr<cv::Mat> get_zeros_map();

  /**
   *  This function is called from inside get_drop_target method based on the item to be dispensed.
   *  Input: boolean describing if ALL items in bag should not be dropped onto
   *  Input: a vector of ints representing item materials that the item cannot be dropped onto
   * Input: number of layers to consider (should be between 1 and num_channels)
   * Output: cv::Mat with 0 = no damage risk, 1 = damage risk based on the input layers_to_include
   * Note: if there are any risk regions of the cv::Mat, the flag risk_present will be set to 1
   */
   std::shared_ptr<cv::Mat> get_risk_map(bool avoid_all_items, std::vector<int> risk_materials, int layers_to_include);

   /**
    * @param Expands the risk map from grid cells that include a risk item (red in visualization) to grid cells
    *        that would lead to collision (yellow in visualization) with risk item if the current dimension item is dropped there
    * @param shadow_length (item shadow dimension. Length = in line with cv::Mat cols)
    * @param shadow_width  (item shadow dimension. Width = in line with cv::Mat rows)
    * @param shadow_height --> this comes into consideration due to item swing (item length becomes shadow height)
    * @return
    */
   std::shared_ptr<cv::Mat> expand_risk_map(std::shared_ptr<cv::Mat> risk_map, float grid_width, float grid_length, float shadow_length,
                           float shadow_width, float shadow_height);

   /**
   *  width = in line with grid rows (bag coordinate axis: x)
   *  length = in line with grid cols (bag coordinate axis: y)
   *  note: candidate data is provided in LFB coordinates, with (0,0) in center of LFB, requires coordinate translation
   */
   std::shared_ptr<Eigen::Matrix3Xd> apply_risk_filter(float grid_width, float grid_length,
                          std::shared_ptr<Eigen::Matrix3Xd> candidate_data, cv::Mat risk_map);

   std::shared_ptr<fulfil::dispense::drop::PackingState> packing_state = nullptr;

   //variable for tracking how many times individual bag has been rejected at any VLS for damage avoidance reasons
   int num_damage_rejections = 0;

   bool risk_present = false; //flag for if the risk map is populated (true) or all 0s (false)

  int parse_in_values(std::shared_ptr<CvBagState> doc);
 private:
  bool raw_mongo_doc_has_been_updated = false;

  int num_rows;
  int num_cols;
  int num_channels;
  int damage_buffer_width;
  float damage_buffer_length;
  float damage_swing_factor;

  // General base info
  std::string collection_name;
  std::string db_name;
  ff_mongo_cpp::mongo_objects::MongoObjectID oid;
  ff_mongo_cpp::mongo_objects::MongoObjectID bag_id;

  // outer vector: the layers of the item map. 0 = top later of bag, 1 = next deeper, etc.
  // inner vectors: of size num_rows * num_cols to represent grid. Order is from top left to bottom right
  // increasing rows first, and then columns
  std::vector<std::vector<int>> item_map_arrays;

  //flag to indicate if the state has changed and requires Mongo database to be updated
  bool update_required;

  /**
  *  Recursive function that adds an item w/ value at a row and col location on the item map, and adjusts
   *  deeper map levels accordingly
  */
  void update_item_entry(int row, int col, int channel, int new_value);

};

} // namespace fulfil

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_MONGO_BAG_STATE_H_
