//
// Created by amber on 10/12/20.
//

#include "Fulfil.Dispense/tray/tray_lane_result.h"
using fulfil::dispense::tray::TrayLaneAggregator;
using fulfil::utils::Logger;



TrayLaneAggregator::TrayLaneAggregator(int error_code)
{
  this->error_codes->insert(error_code);
  this->fatal_error = bool(error_code);
}

TrayLaneAggregator::TrayLaneAggregator(int lane_index, int error_code)
{
    this->error_codes->insert(error_code);
    this->lane_index = lane_index;
    this->aggregated_data = std::make_shared<nlohmann::json>(nlohmann::json::object({
        {"Errors", {error_code}},
        {"Index", lane_index}}));
    this->fatal_error = false;
}

TrayLaneAggregator::TrayLaneAggregator(int lane_index, std::shared_ptr<nlohmann::json> additional_fields, int error_code)
{
  this->lane_index = lane_index;
  this->error_codes->insert(error_code);
  this->aggregated_data->merge(std::move(additional_fields));
  this->aggregated_data->["Index"] = lane_index;
  this->aggregated_data->["Errors"] = { error_code };

}

TrayLaneAggregator::TrayLaneAggregator(int lane_index, nlohmann::json&& additional_fields, int error_code)
{
    this->lane_index = lane_index;
    this->error_codes->insert(error_code);
    this->aggregated_data = std::make_shared<nlohmann::json>(std::move(additional_fields)); // Move or forward?
    this->aggregated_data->["Index"] = lane_index;
    this->aggregated_data->["Errors"] = { error_code };
}

//return iterator type
std::set<int> TrayLaneAggregator::get_errors() const { return *this->error_codes; }

bool TrayLaneAggregator::invalid() const { return this->fatal_error; }

int TrayLaneAggregator::get_lane_id() const { return this->lane_index; }


std::shared_ptr<nlohmann::json> TrayLaneAggregator::encode_lane() {
    if (this->fatal_error)
        return std::make_shared<nlohmann::json>(nlohmann::json::object({}));

    // TODO right now there are no lane basis error codes so hard code success
    (*this->aggregated_data)["Errors"] = this->error_codes->size() > 1 && this->error_codes->contains(0) ?
            nlohmann::json::array(std::move(this->error_codes->begin() + 1), std::move(this->error_codes->end())) :
                nlohmann::json::array({0});
    this->error_codes = nullptr;
    this->fatal_error = true;
    (*this->aggregated_data)["Index"] = nlohmann::json(this->lane_index);
    return this->aggregated_data; //swap?
}