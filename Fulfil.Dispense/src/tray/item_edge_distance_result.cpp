//
// Created by amber on 03/02/22.
//

#include <Fulfil.Dispense/tray/item_edge_distance_result.h>
//#include <Fulfil.Dispense/tray/tray_parser.h>


using fulfil::dispense::tray::ItemEdgeDistanceResult;
using fulfil::utils::Logger;


float ItemEdgeDistanceResult::scale_and_round(float measurement, float scale)
{
    double intermediate = std::round(measurement * scale);
    if (intermediate < 0) intermediate = -1;
    return intermediate;
}

ItemEdgeDistanceResult::ItemEdgeDistanceResult(int error_code)
{
    this->lane_index = -1;
    this->item_edge_distance = -1;
    this->error_code = error_code > 0 ? error_code : 5;
    this->fatal_error = true;

}

ItemEdgeDistanceResult::ItemEdgeDistanceResult(int lane_index, float item_edge_distance, float scale, int error_code)
{
    this->lane_index = lane_index;
    this->item_edge_distance = scale_and_round(item_edge_distance, scale);
    this->error_code = error_code;
    this->fatal_error = false;


}

int ItemEdgeDistanceResult::get_error_code() const
{
    return this->error_code;
}

int ItemEdgeDistanceResult::get_lane_id() const
{
    return this->lane_index;
}

void ItemEdgeDistanceResult::update_item_edge_distance(float distance) {
    this->item_edge_distance = distance;
}

float ItemEdgeDistanceResult::get_item_edge_distance() const { return this->item_edge_distance ; }

std::shared_ptr<nlohmann::json> ItemEdgeDistanceResult::encode_all()
{
    std::shared_ptr<nlohmann::json> result_json = std::make_shared<nlohmann::json>();

    if ( this->fatal_error )
        return std::make_shared<nlohmann::json>(nlohmann::json::object({{"Error", this->error_code}}));

    return std::make_shared<nlohmann::json>(
            nlohmann::json::object({{"Error", this->error_code},
                                    {"Index", this->lane_index},
                                    {"Item_Distance_To_Edge", this->item_edge_distance},
                                   }));

}



