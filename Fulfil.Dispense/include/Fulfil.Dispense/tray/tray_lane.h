//
// Created by steve on 5/19/20.
//

#ifndef FULFIL_DISPENSE_SRC_TRAY_TRAY_LANE_H_
#define FULFIL_DISPENSE_SRC_TRAY_TRAY_LANE_H_

#include <memory>
#include <json.hpp>
#include "Fulfil.Dispense/commands/parsing/tray_parser.h"


namespace fulfil
{
namespace dispense {
namespace tray
{
/**
 * The purpose of this class is to define parameters for items in a lane of a tray
 */
class TrayLane
{
 public:

    TrayLane(int lane_index, float item_height, float item_length,
             float item_width, int num_items, bool rigid, bool has_tongue);

    explicit TrayLane(const dimensional_info::LaneInformation &lane);

    TrayLane()=default;

    [[nodiscard]] bool is_valid() const;

    [[nodiscard]] int lane_id() const;

    [[nodiscard]] float get_item_height_in_meters() const;

    [[nodiscard]] float get_item_length_in_meters() const;

    [[nodiscard]] float get_item_width_in_meters() const;

    [[nodiscard]] int get_num_items() const;

    [[nodiscard]] bool is_rigid() const;

    [[nodiscard]] bool has_tongue() const;

  private:
    // TODO temp transition hybrid
    dimensional_info::LaneInformation m_lane_info{};

    /*
    bool valid;
    void fill_invalid_lane();
    int lane_index;
    float item_height;
    float item_length;
    float item_width;
    int num_items;
    bool rigid;
     */

};
} // namespace tray
} // namespace dispense
} // namespace fulfil


#endif //FULFIL_DISPENSE_SRC_TRAY_TRAY_LANE_H_
