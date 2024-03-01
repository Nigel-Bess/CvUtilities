//
// Created by steve on 5/19/20.
//

#include "Fulfil.Dispense/tray/tray_lane.h"
#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.Dispense/tray/measurement_helpers.h>


using fulfil::utils::Logger;
using fulfil::dispense::tray::TrayLane;


TrayLane::TrayLane(int lane_index, float item_height, float item_length,
                   float item_width, int num_items, bool rigid, bool has_tongue)
{
    dimensional_info::LaneIndex lane_index_type {lane_index};
    dimensional_info::ItemDimensions item_dimensions{item_length, item_width, item_height};
    dimensional_info::ItemPackaging item_package_info {rigid, false, 0};
    dimensional_info::ItemInformation item_info{item_dimensions, item_package_info};

    this->m_lane_info = dimensional_info::LaneInformation(lane_index_type, item_info, num_items, has_tongue);
}

TrayLane::TrayLane(const dimensional_info::LaneInformation& lane) : m_lane_info{lane} {
    if (!this->is_valid()) {
        Logger::Instance()->Debug("Lane has items but complete dimension information not provided.\nHeight = {}, Length = {}, Width = {}",
                              this->m_lane_info.m_item_info.m_item_dimensions.m_H,
                              this->m_lane_info.m_item_info.m_item_dimensions.m_L,
                              this->m_lane_info.m_item_info.m_item_dimensions.m_W);
        //this->m_lane_info.m_lane_index.m_val = -1;
    }
    // removed hard coded bounds check on num items. Should be handled by VLSg
}

 bool TrayLane::is_valid() const
{// Todo make valid checker for lane info specifically, to cover more cases than invalid idx
    bool dimension_valid = (this->m_lane_info.m_item_info.m_item_dimensions.m_H > 0 &&
                          this->m_lane_info.m_item_info.m_item_dimensions.m_L > 0 &&
                          this->m_lane_info.m_item_info.m_item_dimensions.m_W > 0);
    Logger::Instance()->Trace("Valid check in tray lane:\n\tValid index: {}\n\tItem Dimension Obj: {}\n\tItem Counts: {}",
                              this->m_lane_info.m_lane_index.is_valid(),
		    this->m_lane_info.m_item_info.m_item_dimensions.is_valid() || dimension_valid,
		    this->m_lane_info.m_item_count > 0);
    if (dimension_valid != this->m_lane_info.m_item_info.m_item_dimensions.is_valid()) {
         Logger::Instance()->Debug("Item Dimension Obj ({}) and local TrayLane dimension ({}) check mismatch",
		    this->m_lane_info.m_item_info.m_item_dimensions.is_valid(), dimension_valid);
    }
    return (this->m_lane_info.m_lane_index.is_valid() &&
              (dimension_valid || this->m_lane_info.m_item_info.m_item_dimensions.is_valid())
              && this->m_lane_info.m_item_count > 0  );
}

int TrayLane::lane_id() const { return this->m_lane_info.m_lane_index.m_val; }

float TrayLane::get_item_height_in_meters() const {
    return fulfil::measure::to_meters(this->m_lane_info.m_item_info.m_item_dimensions.m_H);
}

float TrayLane::get_item_length_in_meters() const {
    return fulfil::measure::to_meters(this->m_lane_info.m_item_info.m_item_dimensions.m_L);
}

float TrayLane::get_item_width_in_meters() const {
    return fulfil::measure::to_meters(this->m_lane_info.m_item_info.m_item_dimensions.m_W);
}


int TrayLane::get_num_items() const {
    return this->m_lane_info.m_item_count;
}

bool TrayLane::is_rigid() const {
    return bool(this->m_lane_info.m_item_info.m_item_package_info.m_rigid);
}

bool TrayLane::has_tongue() const {
    return this->m_lane_info.m_has_tongue;
}

