//
// Created by amber on 10/12/20.
//

#ifndef FULFIL_DISPENSE_TRAY_RESULT_H
#define FULFIL_DISPENSE_TRAY_RESULT_H

#include <Fulfil.Dispense/tray/tray_lane.h>
#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.CPPUtils/inih/INIReader.h>
#include <Fulfil.Dispense/tray/item_edge_distance_result.h>
//#include <Fulfil.Dispense/tray/tray_parser.h>


namespace fulfil::dispense::tray{
/**
* The purpose of this class is to contain all of the desired
* information about the result of a tray item edge distance and height request
*/
    class TrayResult {
        private:

            /** Converts the value in meters to a value in mm units and rounds  *
            * @param double value in meter units
            * @return rounded value to the nearest unit mm
            */
            float to_rounded_millimeters(float meters);



            /**
             * Get the output (or error) codes that should be returned with the lane results related to item distance to edge
             * */

        public:

            /**
             * TrayResult constructor that initializes the request id with
             * the given request id and marks the result as a failure.
             * @param request_id pointer to string with request id.
             */
            explicit TrayResult(std::shared_ptr<std::string> request_id, int error_code);

            /**
             * TrayResult constructor that takes in the LaneResults from the tray algorithm
             */
            TrayResult(int fed_result,
                       std::shared_ptr<std::string> request_id);

            /**
             * TrayResult constructor that takes in the LaneResults from the tray algorithm
             */
            TrayResult(std::shared_ptr<nlohmann::json> count_result,
                       std::shared_ptr<std::string> request_id, int error_code = 0);

            TrayResult(std::shared_ptr<nlohmann::json> count_result, int fed_result,
                       std::shared_ptr<std::string> request_id, int error_code = 0);

            std::shared_ptr<nlohmann::json> encode_all();

            int get_num_lane_results();
            int get_error_code();

            int fed_result;
            std::shared_ptr<nlohmann::json> count_result;

            /**
             * The id of the request.
             */
            std::shared_ptr<std::string> request_id;

            int get_first_item_edge_distance();
            /**
             * Defines success of tray algorithm
             * (outer most error for the largest issues i.e. no tray camera, bad request parse)
             *  0 = success
             *  > 0 specifies individual errors that occurred during the algorithm
             */
            int success_code;
            /**
             * The output tray algorithm, see TrayAlgorithm for description of LaneResult
             */

            std::string pkid;

    }; // namespace dispense
} // namespace fulfil
#endif //FULFIL_DISPENSE_TRAY_RESULT_H
