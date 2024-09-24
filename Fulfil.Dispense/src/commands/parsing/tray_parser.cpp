//
// Created by Amber on 4/8/22.
//

#include "Fulfil.Dispense/commands/parsing/tray_parser.h"


namespace predicates {
//TODO add Error code specific check Obj
//TODO add Index specific check Obj

    bool is_error_in_array(const nlohmann::json::array_t& err_array) {
        if (err_array.empty()) return true;
        auto errors = [](int i){ return i > 1; };
        auto has_error = std::any_of(err_array.cbegin(),
                                     err_array.cend(), errors);
        return has_error;
    }


    bool is_valid_lane_merge(const char* match_key, const nlohmann::json& lane1,
                             const nlohmann::json& lane2) {
        auto equal_lane_index = [&match_key](auto l1, auto l2) { return (l1[match_key] == l2[match_key]); };
        auto both_lanes = [&lane1, &lane2]() { return lane1.is_array() && lane2.is_array(); };
        auto both_objects = [&lane1, &lane2]() { return lane1.is_object() && lane2.is_object(); };
        return both_lanes() && std::equal (lane1.begin(),
                                           lane1.end(), lane2.begin(), equal_lane_index) ||
               (both_objects() && equal_lane_index(lane1, lane2));
    }

}




namespace dimensional_info {
    LaneIndex::LaneIndex(int rhs) : m_val{rhs} {}

    bool LaneIndex::is_valid() const { return (m_val > -1); }

    ItemDimensions::ItemDimensions(float L, float W, float H) : m_L{L}, m_W{W}, m_H{H} {}

    bool ItemDimensions::is_valid() const {

        return  bool(m_H > 0.0 && m_L > 0.0 && m_W > 0.0);
    }


    ItemPackaging::ItemPackaging(int rigid, bool shiny, int material) :
            m_rigid{rigid}, m_shiny{shiny}, m_material{material} {}

    ItemInformation::ItemInformation(ItemDimensions dims, ItemPackaging package_info) :
            m_item_dimensions{dims}, m_item_package_info{package_info}{}

    LaneInformation::LaneInformation(LaneIndex lane_index, ItemInformation item_info, int item_count, bool has_tongue)
            :m_lane_index{ lane_index }, m_item_info{ item_info }, m_item_count{item_count}, m_has_tongue{has_tongue} {}

    float LaneInformation::expected_lane_height() const {
      constexpr float tray_inset = 25;
      float tongue_adjust = ((m_has_tongue) ? 8 : 0);
      return m_item_info.m_item_dimensions.m_H + tongue_adjust - tray_inset;
    }

    TrayRecipe::TrayRecipe(std::vector<double> lane_centers, int lane_count,
                           float max_item_width, float min_item_width, std::string name) :
            m_lane_center_locs{lane_centers}, m_name{name}, m_max_item_width{max_item_width}, m_min_item_width{min_item_width},
            m_lane_count{lane_count} {}

    bool TrayRecipe::is_valid() const { return m_lane_center_locs.size() == m_lane_count; }

}// namespace dimensional_info


namespace comms_context {

    OidString::OidString(std::string rhs) : m_val{rhs} {
        if (!is_valid()) { to_null(); }
    }

    OidString::OidString(const nlohmann::json& rhs) : m_val{rhs.get<std::string>()} {
        if (!rhs.is_string() || !is_valid()) {
            to_null();
        } else {
            m_val = rhs;
        }
    }

    nlohmann::json OidString::to_mongo_format() const {
        return json_parser::mongo_utils::id_str_to_mongo_oid_json(m_val);
    }

    void OidString::to_null() { m_val = std::string(json_parser::mongo_utils::make_null_id_str()); }

    bool OidString::is_null_id() const { return (m_val == std::string(json_parser::mongo_utils::make_null_id_str())); }

    bool OidString::is_valid() const
    {
        const size_t oid_length = 24;
        auto is_not_number_or_lower_case = [](const char &c) {
            const char zero_char = 48;
            const char nine_char = 57;
            const char a_char = 97;
            const char z_char = 122;
            return bool(c < zero_char || c > z_char || (c > nine_char && c < a_char));
        };

        return !(m_val.size() != oid_length || std::any_of(m_val.cbegin(), m_val.cend(), is_not_number_or_lower_case));
    }

    RequestContextInfo::RequestContextInfo(
            int requestType,
            int calibrationMode,
            bool isAtInduction,
            const comms_context::OidString &primaryKeyId,
            const comms_context::OidString &trayId,
            const std::string &imagePrefix,
            const std::string &imageTag)
            :   m_request_type{requestType}, m_calibration_mode{calibrationMode}, m_is_at_induction{isAtInduction}, m_primary_key_id{primaryKeyId},  m_tray_id{trayId},
                m_image_prefix{imagePrefix},  m_image_tag{imageTag} {}

    const char *RequestContextInfo::get_request_name() const
    {
        auto get_request_name_idx = [](int req_type) {
            if (req_type == 5 || req_type == 15) { return 1; }
            if (req_type == 6 || req_type == 16) { return 2; }
            return 0;
        };
        int name_idx = get_request_name_idx(m_request_type);
        return RequestName.at(name_idx);
    }

    const char *RequestContextInfo::get_calibration_mode_key() const
    {
        return CalibrationKeyName.at(m_calibration_mode);
    }

    const char *RequestContextInfo::get_tray_position_name() const
    {
        return TrayPositionNames.at(m_calibration_mode);
    }

    std::string RequestContextInfo::get_id_tagged_sequence_step() const
    {
        return json_parser::mongo_utils::make_sequence_step_tag(m_image_prefix,
                                                              std::string(get_request_name()), get_primary_key_id());
    }

    std::string RequestContextInfo::get_sequence_step() const
    {
        return json_parser::mongo_utils::make_sequence_step_tag(m_image_prefix, get_request_name(), "");
    }

    std::string RequestContextInfo::get_primary_key_id() const {
        return m_primary_key_id.m_val;
    }

    std::string RequestContextInfo::to_string() const {
        return std::string("{ Request ") + get_request_name() + "PKID: " + get_primary_key_id() + " for Tray " + m_tray_id.m_val + " and sequence step " + get_sequence_step() + " }";
    }

//TODO change to class and find less repetitious way to implement.
    nlohmann::json RequestContextInfo::to_mongo_format() const {
        auto mongo_formatted_request_info = nlohmann::json{
                { "Type", m_request_type },
                { "Primary_Key_ID", m_primary_key_id.to_mongo_format() },
                { "Image_Prefix", m_image_prefix },
                { "Request_Name", get_request_name() },// TODO delete?
                { "Tray_ID", m_tray_id.to_mongo_format() },
                { "Calibration_Mode", m_calibration_mode },
                { "Is_At_Induction", m_is_at_induction || false },
                { "Tray_Position", get_tray_position_name() },
                { "Image_Tag", (m_image_tag.empty()) ? get_id_tagged_sequence_step() : m_image_tag}
        };
        return mongo_formatted_request_info;
    }

}// namespace comms_context



namespace request_from_vlsg {
    using namespace dimensional_info;
    TrayRequest::TrayRequest(const comms_context::RequestContextInfo &reqContext,
                             const std::vector<dimensional_info::LaneInformation>& trayLanes,
                             dimensional_info::TrayRecipe trayRecipe) : m_context{reqContext}, m_lanes{trayLanes}, m_tray_recipe{trayRecipe} {}

    float TrayRequest::expected_max_height() const {
      auto max_item_height = std::max_element(m_lanes.begin(), m_lanes.end(), []
          (dimensional_info::LaneInformation lane1, dimensional_info::LaneInformation lane2)
          { return lane1.expected_lane_height() < lane2.expected_lane_height(); });
        return max_item_height->expected_lane_height();
    }


    std::string TrayRequest::get_primary_key_id() const {
        return m_context.get_primary_key_id();
    }

    std::string TrayRequest::get_sequence_step() const {
        return m_context.get_sequence_step();
    }

    std::string TrayRequest::to_string() const {
        return m_context.to_string();
    }

}// namespace request_from_vlsg



namespace results_to_vlsg {

//TODO: Free and template. We will actually want to clip both distance and number of items in lane etc

    BoundaryLimit::BoundaryLimit(float min_distance, float max_distance) :
            m_min_distance{min_distance}, m_max_distance{max_distance} {}

    int BoundaryLimit::clip(int distance) const
    {
        return (distance < m_max_distance && distance > m_min_distance) ? distance : -1;
    }
    LaneItemDistance::LaneItemDistance(dimensional_info::LaneIndex index, std::vector<int> errors,
                                       int firstItemDistance,
                                       BoundaryLimit clip) : m_index{index}, m_errors{std::move(errors)},
                                                             m_first_item_distance{firstItemDistance}, m_range_fn{clip} {}

    LaneItemDistance::LaneItemDistance(int index, int error, int firstItemDistance) : m_index{index},
                                                                                      m_errors{std::vector<int>{error}},
                                                                                      m_first_item_distance{firstItemDistance} {}

    LaneItemDistance::LaneItemDistance(int index, int error, int firstItemDistance, int firstItemLength) : m_index{index},
                                                                                                           m_errors{std::vector<int>{error}},
                                                                                                           m_first_item_distance{firstItemDistance},
                                                                                                           m_first_item_length{firstItemLength}{}


    dimensional_info::LaneIndex LaneItemDistance::get_lane_id() const { return m_index; }

    LaneCounts::LaneCounts(dimensional_info::LaneIndex index, std::vector<int> errors, int laneCounts, bool has_tongue)
        : m_index{index}, m_errors{std::move(errors)}, m_num_algorithm_counts{laneCounts} {}

    LaneCounts::LaneCounts(int index, int error, int laneCounts, bool has_tongue, bool new_has_tongue, bool new_has_spacer, int counts)
        : m_index{index}, m_errors{std::vector<int>{error}},  m_num_algorithm_counts{laneCounts}, m_new_has_tongue{new_has_tongue}, m_new_has_spacer{new_has_spacer}, m_counts{counts} {}

    dimensional_info::LaneIndex LaneCounts::get_lane_id() const { return m_index; }

    TrayHeight::TrayHeight(float max_detected_height, float max_item_height_in_tray) :
              m_max_height{ max_detected_height },
              m_confidence{ confident_measurement(max_detected_height, max_item_height_in_tray) } {}

    bool TrayHeight::confident_measurement(float max_detected_height, float max_item_height_in_tray) const {
      return (max_detected_height > 0) && (max_detected_height < max_item_height_in_tray * 1.4);
    }

    //TrayHeight::TrayHeight(float max_height, float max_item_height)

    TrayValidationCounts::TrayValidationCounts(std::vector<int> laneErrors,
                                               std::vector<results_to_vlsg::LaneCounts> algoLaneCounts, int dispenseCount) : m_errors{ laneErrors}, m_lanes{algoLaneCounts}, m_dispense_count{dispenseCount} {}
    void TrayValidationCounts::update_lane_tongue_detections(std::vector<bool> tongue_detections) {
      std::transform(m_lanes.begin(), m_lanes.end(), tongue_detections.cbegin(),
          m_lanes.begin(), [&](results_to_vlsg::LaneCounts lane, bool detection) {
            lane.m_has_tongue = detection;
            return lane;} );

    }

    nlohmann::json &LaneItemDistance::to_tray_format_json(nlohmann::json &j)
    {
        auto lane_index_ref = j.find("Index");
        int lane_index = -1;
        if (lane_index_ref != j.end()) {
            lane_index = *lane_index_ref;
            j.erase(lane_index_ref);
        }
        j["Lanes"] = nlohmann::json::array({ dimensional_info::LaneIndex{ lane_index } });
        if (lane_index < 0) { j["Errors"] = std::vector<int>({5}); }
        return j;
    }


}// namespace results_to_vlsg



namespace tray_count_api_comms {
    BoundingBoxInfo::BoundingBoxInfo(int classIdx, dimensional_info::LaneIndex laneId,
                                     float score, float xmax, float xmin, float ymax, float ymin, int error)
            : m_lane_id{laneId}, m_score{score}, m_xmax{xmax}, m_xmin{xmin}, m_ymax{ymax},
              m_ymin{ymin}, m_class_idx{classIdx}, m_error{error}
    {}

    LaneImageRegion::LaneImageRegion(std::vector<cv::Point2i> roi, float yscale, float xscale)
    : m_yscale(yscale), m_xscale(xscale){
        if (roi.size() == 4) {
            for (int i = 0 ; i < 4; i++) {
                vertices[2*i] = roi[i].y/m_yscale;
                vertices[2*i + 1] = roi[i].x/m_xscale;
            }
        } else {
            m_yscale = m_xscale = 0;
        }

    }
}// namespace tray_count_api_comms


namespace tray_count_api_comms {
    LaneCenterLine::LaneCenterLine(float yFront, float xFront, float yBack, float xBack)
            : m_front_y{yFront}, m_front_x{xFront}, m_back_y{yBack}, m_back_x{xBack} {}
}// namespace tray_count_api_comms


namespace tray_count_api_comms {

    std::string DetectionAPIResults::extract_model_version_from_path() const
    {
        std::string model_version_name = "";
        if (!m_used_model_path.empty()) {
            std::string::size_type path_end =
                    (m_used_model_path.back() == '/') ? m_used_model_path.size() - 1 : m_used_model_path.size() - 0;
            // '/saved_model_format/saved_model' is 31 char
            std::string_view model_dir = "/models/";
            std::string::size_type found = m_used_model_path.rfind(model_dir, path_end);
            if (found != std::string::npos) {
                std::string::size_type model_ver_start = found + model_dir.length();
                std::string::size_type model_ver_end = m_used_model_path.find('/', model_ver_start);
                model_version_name = m_used_model_path.substr(model_ver_start, model_ver_end - model_ver_start);
            } else
                std::cerr << "Issue finding version!\n";
        }
        return model_version_name;
    }

    DetectionAPIResults::DetectionAPIResults(std::vector<tray_count_api_comms::BoundingBoxInfo> boundingBoxes,
                                             std::vector<tray_count_api_comms::LaneCenterLine> centerLines,
                                             int inputImageHeight,
                                             int inputImageWidth,
                                             std::string modelVersion,
                                             std::string imageFilePath,
                                             float shrinkFactor,
                                             std::string usedModelPath,
                                             int statusCode)
            : m_bounding_boxes {std::move(boundingBoxes)}, m_center_lines{std::move(centerLines)}, m_model_version{std::move(modelVersion)},
              m_image_file_path{std::move(imageFilePath)}, m_used_model_path{std::move(usedModelPath)}, m_shrink_factor{shrinkFactor},
              m_input_image_height{inputImageHeight}, m_input_image_width{inputImageWidth}, m_status{statusCode} {}
}// namespace tray_count_api_comms


namespace mongo_records {
    TrayCountRecord::TrayCountRecord(results_to_vlsg::TrayValidationCounts trayCount,
                                     tray_count_api_comms::DetectionAPIResults detectionData,
                                     comms_context::RequestContextInfo context,
                                     comms_context::OidString calibrationId,
                                     std::string vlsName) : m_tray_count{std::move(trayCount)}, m_detections{std::move(detectionData)},
                                                            m_context{std::move(context)}, m_calibration_id{std::move(calibrationId)},
                                                            m_vls_name{vlsName} {}

    std::string TrayCountRecord::get_primary_key_id() const {
        return m_context.get_primary_key_id();
    }


}// namespace mongo_records