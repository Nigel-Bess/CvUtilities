//
// Created by steve on 1/21/21.
//

#include "Fulfil.Dispense/mongo/mongo_bag_state.h"
#include <Fulfil.CPPUtils/logging.h>

using fulfil::mongo::MongoBagState;
using fulfil::utils::Logger;
using ff_mongo_cpp::mongo_objects::MongoObjectID;



MongoBagState::MongoBagState()
{
  Logger::Instance()->Info("Starkey says this constructor SHOULD be used!"); // TODO why ? is this up to date? context needed
  this->collection_name = "BagStates";
  this->db_name = "Inventory";
  this->update_required = false;
}

MongoBagState::MongoBagState(MongoObjectID bag_id, bool new_doc)//default to 3.1
{
  this->collection_name = "BagStates";
  this->db_name = "Inventory";
  this->bag_id = bag_id;
  if(new_doc) CreateDefaultMap();
  this->update_required = false;
// TODO should packing state be here


    //default to Bag3
  this->num_rows = 8;
  this->num_cols = 6;
  this->num_channels = 3;
}

void MongoBagState::CreateDefaultMap()
{
  std::vector<std::vector<int>> item_map_layers;
  std::vector<int> item_array_layer(num_rows * num_cols, 0);
  for(int i=0; i < num_channels; i++) item_map_layers.push_back(item_array_layer);
  this->item_map_arrays = item_map_layers;
}


void MongoBagState::SetObjectID(MongoObjectID oid)
{
  this->oid = oid;
}

MongoObjectID MongoBagState::GetObjectID()
{
  return this->oid;
}

std::string MongoBagState::GetCollection()
{
  return this->collection_name;
}

std::string MongoBagState::GetDataBase()
{
  return this->db_name;
}

void MongoBagState::SetCollection(const std::string& new_collection_name)
{
  this->collection_name = new_collection_name;
}

void MongoBagState::SetDataBase(const std::string& new_db_name)
{
  this->db_name = new_db_name;
}

/*
std::shared_ptr<ff_mongo_cpp::mongo_objects::MongoDocument>
    MongoBagState::MakeNewMongoDocument(bsoncxx::document::view doc,
                                        const std::string& collection_name,
                                        const std::string& db_name)
{
  Logger::Instance()->Error("MakeNewMongoDocument method should currently not be used!");
  std::shared_ptr<MongoBagState> new_doc = std::make_shared<MongoBagState>();
  return new_doc;
}
*/

int MongoBagState::parse_in_values(std::shared_ptr<CvBagState> bag) {
  try
  {
    raw_mongo_doc = bag;
    this->bag_id = raw_mongo_doc->BagId;
    this->oid = bag->MongoID;
    std::vector<std::vector<int>> new_arrays; //reset to empty vector
    
    this->num_rows = bag->Config->grid_rows;
    this->num_cols = bag->Config->grid_cols;
    this->damage_buffer_width = bag->Config->damage_buffer_width;
    this->damage_buffer_length = bag->Config->damage_buffer_length;
    this->damage_swing_factor = bag->Config->damage_swing_factor;


    // if FC sends empty arrays, populate with 0s
    if(bag->ItemMap1.size() == 0) {
        for (auto cr = 0; cr < num_rows * num_cols; cr++) {
            bag->ItemMap1.push_back(0);
            bag->ItemMap2.push_back(0);
            bag->ItemMap3.push_back(0);
        }
    }

    new_arrays.push_back(bag->ItemMap1);
    new_arrays.push_back(bag->ItemMap2);
    new_arrays.push_back(bag->ItemMap3);
    this->item_map_arrays = new_arrays;

    this->packing_state = std::make_shared<dispense::drop::PackingState>(bag->Config->LFB_cavity_height,
                                                                         bag->Config->container_width,
                                                                         bag->Config->container_length,
                                                                         bag->Config->max_num_depth_detections);
    this->packing_state->set_packed_items_volume_mm(bag->PackedItemsVolume);

    this->num_damage_rejections = bag->NumberDamageRejections;

    this->num_channels = bag->Config->grid_channels;
  }
  catch (std::exception& e)
  { // We should start moving away from excessive catch throws
    Logger::Instance()->Warn("Unable to parse values from the Bag object.\n  Exception:\n\t{}", e.what());
    throw;
  }
  return 0;
}

/*
bsoncxx::document::value MongoBagState::MakeWritableValue() {
  //TODO: add check that all fields are populated and available!
  Logger::Instance()->Debug("Making Writable Value in BagState MongoDoc now");
  MongoBsonxxEncoder bag_encoder = MongoBsonxxEncoder();
  bag_encoder.addField("Bag_id", this->bag_id);
  bag_encoder.addField("ItemMap1", this->item_map_arrays[0]);
  bag_encoder.addField("ItemMap2", this->item_map_arrays[1]);
  bag_encoder.addField("ItemMap3", this->item_map_arrays[2]);
  bag_encoder.addField("PackedItemsVolume", this->packing_state->get_packed_items_volume_mm());
  bag_encoder.addField("PercentBagFull", this->packing_state->get_percent_bag_full());
  bag_encoder.addField("PackingEfficiency", this->packing_state->get_packing_efficiency());
  bag_encoder.addField("NumberDamageRejections", this->num_damage_rejections);
  // TODO - do we want to add LfbVisionConfiguration? my heart says yes, but says no because it's literally already in Mongo...
  bsoncxx::document::value final_doc = bag_encoder.extract();
  return final_doc;
}
*/

void MongoBagState::UpdateRawMongoDocState()
{
  std::copy(std::begin(item_map_arrays[0]), std::end(item_map_arrays[0]), std::begin(raw_mongo_doc->ItemMap1));
  std::copy(std::begin(item_map_arrays[1]), std::end(item_map_arrays[1]), std::begin(raw_mongo_doc->ItemMap2));
  std::copy(std::begin(item_map_arrays[2]), std::end(item_map_arrays[2]), std::begin(raw_mongo_doc->ItemMap3));

  raw_mongo_doc->PackedItemsVolume = this->packing_state->get_packed_items_volume_mm();
  raw_mongo_doc->PercentBagFull = this->packing_state->get_percent_bag_full();
  raw_mongo_doc->PackingEfficiency = this->packing_state->get_packing_efficiency();
  raw_mongo_doc->NumberDamageRejections = this->num_damage_rejections;
  // TODO - is LfbVisionConfig needed here
}

nlohmann::json MongoBagState::GetStateAsJson() //bool include_lfb_vision_config)
{
  // gating on if the bag state hasn't been generated yet for offline test backwards compatibility
  if (!this->bag_id.is_null())
  {
      this->UpdateRawMongoDocState();
      return raw_mongo_doc->ToJson(); //include_lfb_vision_config);
  }
  return nlohmann::json();
}

std::string MongoBagState::GetStateAsString()
{
  this->UpdateRawMongoDocState();
  return raw_mongo_doc->ToString();
}

void MongoBagState::update_item_entry(int row, int col, int channel, int new_value)
{
  if(row >= num_rows or col >= num_cols or channel >= num_channels)
  {
    Logger::Instance()->Error("Incorrect input provided to update_item_entry function in MongoBagState");
    return;
  }
  else if(item_map_arrays.size() != num_channels or item_map_arrays[0].size() != (num_rows * num_cols))
  {
    Logger::Instance()->Error("Item map arrays format in MongoBagState does not match expected. Cannot update entry");
    return;
  }

  int index = row*num_cols + col;
  int current_value = item_map_arrays[channel][index];
  item_map_arrays[channel][index] = new_value;
  Logger::Instance()->Trace("Updated item map value at channel: {}, position: {} from value: {} to value: {}", channel, index, current_value, new_value);

  if(current_value != 0 and (channel + 1) < num_channels) update_item_entry(row, col, channel + 1, current_value); //recursively update deeper layers of map
  return;
}

void MongoBagState::update_item_map(std::shared_ptr<cv::Mat> comparison_result, int material_type)
{
  if(comparison_result == nullptr)
  {
    Logger::Instance()->Debug("Comparison result is nullptr, indicating bag state does not need updating.");
    this->update_required = false;
  }
  else if(comparison_result->rows != this->num_rows or comparison_result->cols != this->num_cols)
  {
    Logger::Instance()->Error("Comparison result dims don't match mongo bag state dims. Cannot update item map");
    this->update_required = false;
    return;
  }
  else
  {
    Logger::Instance()->Debug("Comparison result is populated. Updating Mongo Bag State Doc with results");
    for(int row = 0; row < num_rows; row++)
    {
      for (int col = 0; col < num_cols; col++)
      {
        int value = comparison_result->at<float>(row, col); //TODO: may want to change the cv::Mat result to storing ints, not floats
        if(value) update_item_entry(row, col, 0, material_type);
      }
    }
    this->update_required = true;
  }
}



std::vector<cv::Mat> MongoBagState::build_cv_item_map()
{
  int num_entries = this->item_map_arrays[0].size();
  Logger::Instance()->Debug("Building item map into cv::Mat now with {} channels", num_channels);

  if ((num_rows * num_cols) != num_entries or num_channels != this->item_map_arrays.size())
  {
    Logger::Instance()->Error("Dims of item_map_arrays do not match requested cv::Mat rows/cols inputs. Cannot build");
    throw std::runtime_error("Failed to build item map cv::Mat based on mongo item map"); //TODO: select different error code here
  }

  std::vector<cv::Mat> item_map; //note: channel 0 is at top of bag, n is at bottom of bag

  for(int channel = 0; channel < num_channels; channel++)
  {
    cv::Mat single_layer = cv::Mat::zeros(num_rows, num_cols, CV_8UC1);
    for(int row = 0; row < num_rows; row++)
    {
      for(int col = 0; col < num_cols; col++)
      {
        int index = row*num_cols + col;
        int value = item_map_arrays[channel][index];

        single_layer.at<uchar>(row,col) = value;
      }
    }
    item_map.push_back(single_layer);
  }
  Logger::Instance()->Info("The shape of the output item map is: {} channels, and  ({},{}) (row,col)",
                            item_map.size(), item_map[0].rows, item_map[0].cols);
  /**
  for(int channel = 0; channel < num_channels; channel++)
  {
    cv::namedWindow("test", cv::WINDOW_NORMAL);
    cv::imshow("test", item_map[channel]);
    cv::resizeWindow("test",800,600);
    cv::moveWindow("test", 0, 0);
    cv::waitKey(0);
  }
  **/

  return item_map;
}

void MongoBagState::set_item_map_arrays_from_mat(std::shared_ptr<std::vector<cv::Mat>> item_map_ptr)
{
  std::vector<cv::Mat> item_map = *item_map_ptr;
  int num_channels = item_map.size();
  int num_rows = item_map[0].rows;
  int num_cols = item_map[0].cols;

  std::vector<std::vector<int>> new_item_map_arrays;

  for(int channel = 0; channel < num_channels; channel++)
  {
    std::vector<int> new_array;
    for(int row = 0; row < num_rows; row++)
    {
      for (int col = 0; col < num_cols; col++)
      {
        int value = item_map[channel].at<uchar>(row, col);
        new_array.push_back(value);
      }
    }
    new_item_map_arrays.push_back(new_array);
  }

  this->item_map_arrays = new_item_map_arrays;

  return;
}


std::shared_ptr<cv::Mat> MongoBagState::get_zeros_map()
{
  return std::make_shared<cv::Mat>(cv::Mat::zeros(num_rows, num_cols, CV_8UC1)); //unsigned int type
}

std::shared_ptr<cv::Mat> MongoBagState::get_risk_map(bool avoid_all_items, std::vector<int> risk_materials, int layers_to_include)
{
  risk_present = false; //reset flag for risk materials being present in bag state

  if (layers_to_include < 1 or layers_to_include > this->num_channels)
  {
    Logger::Instance()->Error("Input number of layers_to_include in risk map does not make sense");
    throw std::runtime_error("Error in creating risk map");
  }
  if (this->item_map_arrays.size() < this->num_channels)
  {
    Logger::Instance()->Error("Item map arrays not populated correctly. Cannot proceed with risk map");
    throw std::runtime_error("Error in creating risk map");
  }

  cv::Mat risk_map = cv::Mat::zeros(num_rows, num_cols, CV_8UC1); //unsigned int type

  //TODO: this can be restructured so that if a region has already been set positive, other layers do not also need to be checked at that region
  for (int i = 0; i < layers_to_include; i++)
  {
    std::vector<int> layer = this->item_map_arrays[i];
    for (int row = 0; row < num_rows; row++)
    {
      for (int col = 0; col < num_cols; col++)
      {
        int value = layer[row * num_cols + col];
        if (value != 0) //if there is an item material present, check if it is in the risk materials list
        {
          if(avoid_all_items)
          {
            risk_map.at<uchar>(row, col) = 1;
            risk_present = true;
          }
          else if (std::find(risk_materials.begin(), risk_materials.end(), value) != risk_materials.end())
          {
            risk_map.at<uchar>(row, col) = 1;
            risk_present = true;
          }
        }
      }
    }
  }
  return std::make_shared<cv::Mat>(risk_map);
}

// drop target wall off
std::shared_ptr<cv::Mat> MongoBagState::expand_risk_map(std::shared_ptr<cv::Mat> risk_map, float grid_width, float grid_length, float shadow_length,
                                       float shadow_width, float shadow_height)
{
  float cell_width = grid_width / num_rows;    //in line with rows (vertical edge)
  float cell_length = grid_length / num_cols;  //in line with columns (horizontal edge)

  // IMPORTANT damage configs
  int num_width_squares = ceil((shadow_width/2) / cell_width) + damage_buffer_width; //number of squares on either side of risk squares that will be ineligible

  //TODO: refactor code so swing is only taken into account depending on grid square relation to limit line (whether LFB will rotate or not).
  //TODO: for now this will be kept simple and always assume swing risk, which is a reasonable assumption if dispensing on either side of an item already in bag
  //int num_length_squares = ceil((shadow_length/2) / cell_length) + damage_buffer_length; //in direction away from VLS (no swing)
  float swing_expansion_amount = std::max((shadow_length/2), damage_swing_factor * shadow_height);
  int num_length_squares = ceil(swing_expansion_amount / cell_length) + this->damage_buffer_length; //in direction toward from VLS (yes swing)

  //update risk map in place
  for(int row=0; row < num_rows; row++)
  {
    for(int col=0; col<num_cols; col++)
    {
      int value = risk_map->at<uchar>(row, col);

      if(value == 1) //if current square is a risk square, expand to surrounding squares based on item dims
      {
        int expanded_top_bound_row = std::max(0, row - num_width_squares);
        int expanded_bottom_bound_row = std::min(num_rows - 1, row + num_width_squares) ;
        int expanded_left_bound_col = std::max(0, col - num_length_squares);
        int expanded_right_bound_col = std::min(num_cols - 1, col + num_length_squares);

        if(expanded_top_bound_row >= expanded_bottom_bound_row or expanded_left_bound_col >= expanded_right_bound_col)
        {
          Logger::Instance()->Error("Expansion bounds in expand_risk_map do not make sense!!! Returning current map");
          return risk_map;
        }

        for(int r_index = expanded_top_bound_row; r_index <= expanded_bottom_bound_row; r_index++)
        {
          for(int c_index = expanded_left_bound_col; c_index <= expanded_right_bound_col; c_index++)
          {
            if(risk_map->at<uchar>(r_index, c_index) == 0) risk_map->at<uchar>(r_index, c_index) = 2;
          }
        }
      }
    }
  }
  return risk_map;
}

std::shared_ptr<Eigen::Matrix3Xd> MongoBagState::apply_risk_filter(float grid_width, float grid_length,
                                      std::shared_ptr<Eigen::Matrix3Xd> candidate_data, cv::Mat risk_map)
{
  Logger::Instance()->Debug("Applying damage risk filter to target candidates of size: {}", candidate_data->cols());

  float cell_width = grid_width / num_rows;    //in line with rows (vertical edge)
  float cell_length = grid_length / num_cols;  //in line with columns (horizontal edge)

  int num_adjusted_points = 0;
  int insert_index = 0;

  std::shared_ptr<Eigen::Matrix3Xd> new_data = std::shared_ptr<Eigen::Matrix3Xd>(new Eigen::Matrix3Xd(3, candidate_data->cols()));

  for(int index=0; index < candidate_data->cols(); index++)
  {
    float x = candidate_data->col(index).x();
    float y = candidate_data->col(index).y();
    float z = candidate_data->col(index).z();

    int row_index = floor((x - (-1 * grid_width/2)) / cell_width);
    int col_index = floor((y - (-1 * grid_length/2)) / cell_length);
    Logger::Instance()->Trace("Row index is: {} and col index is: {}", row_index, col_index);

    if(row_index < 0 or row_index >= num_rows or col_index < 0 or col_index >= num_cols)
    {
      Logger::Instance()->Error("Error in applying risk filter! Candidates outside of expected regions!!");
      return nullptr;
    }

    if(risk_map.at<uchar>(row_index,col_index) != 0)
    {
      Logger::Instance()->Trace("Removing candidate point from consideration because of damage risk");
      num_adjusted_points++;
    }
    else
    {
      (*new_data)(0,insert_index) = x;
      (*new_data)(1,insert_index) = y;
      (*new_data)(2,insert_index) = z;
      insert_index++;
    }
  }
  int remaining_points = candidate_data->cols() - num_adjusted_points;
  Logger::Instance()->Debug("Number of adjusted points is: {}, remaining candidates is: {}", num_adjusted_points, remaining_points);

  if(insert_index != remaining_points) Logger::Instance()->Error("Counting error in risk filter!!");
  new_data->conservativeResize(3, remaining_points); //get rid of
  return new_data;
}
