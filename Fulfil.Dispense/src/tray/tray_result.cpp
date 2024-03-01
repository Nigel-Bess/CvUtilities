//
// Created by amber on 10/12/20.
//

#include <Fulfil.Dispense/tray/tray_result.h>
#include <Fulfil.Dispense/tray/tray_algorithm.h>
#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.Dispense/json.hpp>
#include <Fulfil.Dispense/tray/item_edge_distance_result.h>
//#include <Fulfil.Dispense/tray/tray_parser.h>


using fulfil::dispense::tray_processing::TrayAlgorithm;
using fulfil::dispense::tray::TrayResult;
using fulfil::utils::Logger;


TrayResult::TrayResult(std::shared_ptr<std::string> request_id, int error_code)
{
  this->request_id = request_id;
  this->success_code = error_code;
}


TrayResult::TrayResult(int fed_result,
                       std::shared_ptr<std::string> request_id)
{
  this->fed_result = fed_result;
  (*this->count_result)["First_Item_Distance"] = this->fed_result;
  this->request_id = request_id;
  this->success_code = 0;
}

TrayResult::TrayResult(std::shared_ptr<nlohmann::json> count_result,
                       std::shared_ptr<std::string> request_id, int error_code)
{
  this->count_result = count_result;
  this->fed_result = -1;
  this->request_id = request_id;
  this->success_code = error_code;
}

TrayResult::TrayResult(std::shared_ptr<nlohmann::json> count_result,
                       int fed_result,
                       std::shared_ptr<std::string> request_id, int error_code)
{
    this->count_result = count_result;
    this->fed_result = fed_result;
    (*this->count_result)["First_Item_Distance"] = this->fed_result;
    this->request_id = request_id;
    this->success_code = error_code;
}

int TrayResult::get_num_lane_results()
{
  return this->count_result->at("Lanes").size();
}

int TrayResult::get_first_item_edge_distance(){
    return this->fed_result;
}

int TrayResult::get_error_code()
{
  return 0;
}


std::shared_ptr<nlohmann::json> TrayResult::encode_all()
{
    //std::shared_ptr<nlohmann::json> result_json = std::make_shared<nlohmann::json>();
    //*result_json = nlohmann::json(this->count_result);
  (*this->count_result)["Error"] = this->success_code;
  if (!this->count_result->contains("First_Item_Distance")) {
      (*this->count_result)["First_Item_Distance"] = this->fed_result;
  }
    return this->count_result;

    //if (this->fed_result.get_error_code() > 0)
      //  (*result_json)["Errors"].emplace_back(this->fed_result.get_error_code())
}

