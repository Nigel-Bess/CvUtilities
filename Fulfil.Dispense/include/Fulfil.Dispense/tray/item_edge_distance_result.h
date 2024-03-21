//
// Created by amber on 03/02/22.
//

#ifndef FULFIL_DISPENSE_TRAY_ITEM_EDGE_DISTANCE_RESULT_H_
#define FULFIL_DISPENSE_TRAY_ITEM_EDGE_DISTANCE_RESULT_H_

#include <memory>
#include <vector>
#include <json.hpp>
#include <Fulfil.CPPUtils/logging.h>
#include "Fulfil.Dispense/commands/parsing/tray_parser.h"


namespace fulfil
{
    namespace dispense {
        namespace tray
        {
/**
 * The purpose of this class is to define result of lane analysis
 */
            class ItemEdgeDistanceResult
            {
            private:
                float item_edge_distance;
                int lane_index;
                int error_code;
                bool fatal_error;

            public:
                // TODO move to protected in the virtual
                static float scale_and_round(float measurement, float scale=1000);
                explicit ItemEdgeDistanceResult(int error_code);
                ItemEdgeDistanceResult(int lane_index, float item_edge_distance, float scale=1, int error_code=0);
                [[nodiscard]] int get_error_code() const;
                [[nodiscard]] int get_lane_id() const;

                [[nodiscard]] float get_item_edge_distance() const;
                void update_item_edge_distance(float distance);


                std::shared_ptr<nlohmann::json> encode_all();








            };
        } // namespace tray
    } // namespace dispense
} // namespace fulfil


#endif // FULFIL_DISPENSE_TRAY_ITEM_EDGE_DISTANCE_RESULT_H_
