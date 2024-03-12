//
// Created by amber on 4/1/22.
//

#ifndef FULFIL_DISPENSE_TRAY_PARSER_H
#define FULFIL_DISPENSE_TRAY_PARSER_H
#include <Fulfil.Dispense/tray/mongo_json_utils.h>



namespace predicates {
    bool is_error_in_array(const nlohmann::json::array_t& err_array);

    bool is_valid_lane_merge(const char* match_key, const nlohmann::json& lane1,
                             const nlohmann::json& lane2);

}


/**
 * dimensional_info (from request):
 *      - LaneIndex
 *          - index of lane w/ serial/deserial helpers
 *      - ItemDimensions
 *          - LxWxH of item
 *      - ItemPackaging
 *          - Packaging properties like rigidity, shiny-ness, material
 *      - ItemInformation
 *          - Combines the ItemDimensions & ItemPackaging info for lane
 *      - LaneInformation
 *          - Combines request item counts, LaneIndex, and ItemInformation for lane
 *      - TrayRecipe
 *          - Parses tray recipe obj from request
 *
 * */
namespace dimensional_info {

    struct LaneIndex
    {
        int m_val {-1};
        LaneIndex() = default;
        explicit LaneIndex(int rhs);
        operator int () const { return m_val; }
        constexpr bool operator==(LaneIndex& other) const { return m_val == other.m_val; }
        [[nodiscard]] bool is_valid() const;

    };

    struct ItemDimensions {
        float m_L {0};
        float m_W {0};
        float m_H {0};
        [[nodiscard]] bool is_valid() const;
        ItemDimensions() = default;
        explicit ItemDimensions(float L, float W, float H);
    };

    struct ItemPackaging {
        int m_rigid {0};
        bool m_shiny {false};
        int m_material {0};
        ItemPackaging() = default;
        explicit ItemPackaging(int rigid, bool shiny, int material);
    };

    struct ItemInformation {
        ItemDimensions m_item_dimensions {};
        ItemPackaging m_item_package_info {};
        ItemInformation() = default;
        ItemInformation(ItemDimensions dims, ItemPackaging package_info);

    };

    struct LaneInformation {
        LaneIndex m_lane_index {-1};
        ItemInformation m_item_info{};
        int m_item_count {0};
        bool m_has_tongue {false};
        LaneInformation() = default;
        // has_tongue given def for now due to historical data
        LaneInformation(LaneIndex lane_index, ItemInformation item_info, int item_count, bool has_tongue=false);
        [[nodiscard]] float expected_lane_height() const;
    };

    struct TrayRecipe {
        std::vector<double> m_lane_center_locs {};
        std::string m_name{};
        float m_max_item_width {};
        float m_min_item_width {};
        int m_lane_count {};
        TrayRecipe() = default;
        explicit TrayRecipe(std::vector<double> lane_centers, int lane_count,
                            float max_item_width, float min_item_width, std::string name);

        [[nodiscard]] bool is_valid() const;
    };

    template<typename BasicJsonType>
    void to_json(BasicJsonType &j, const LaneIndex& laneIndex)
    {
        j = (laneIndex.is_valid()) ? nlohmann::json({ { "Index", laneIndex.m_val } }) : nlohmann::json({});
    }

    template<typename BasicJsonType>
    void from_json(const BasicJsonType &j, LaneIndex &laneIndex)
    {
        auto get_index = [&j]() {
            if (j.is_number()) { return j.template get<int>(); }
            if (j.is_object()) { return j.value("Index", -1); }
            return -1;
        };
        laneIndex.m_val = get_index();
    }

    template<typename BasicJsonType>
    void to_json(BasicJsonType &j, const ItemDimensions& itemDims)
    {
        j = { { "L", itemDims.m_L }, { "W", itemDims.m_W }, { "H", itemDims.m_H } };
    }

    template<typename BasicJsonType>
    void from_json(const BasicJsonType &j, ItemDimensions &itemDims)
    {
        try {
            itemDims.m_L = j["L"].template get<float>();
            itemDims.m_W = j["W"].template get<float>();
            itemDims.m_H = j["H"].template get<float>();
        } catch (nlohmann::json::exception &e) {
            std::cerr << "While parsing Item dimensions, encountered exception with id: " << e.id
                      << " and the following message:\n"
                      << e.what() << '\n';
        }
    }

    template<typename BasicJsonType>
    void to_json(BasicJsonType &j, const ItemPackaging& itemPackaging)
    {
        j = {
                { "Rigid", itemPackaging.m_rigid },
                { "Shiny", itemPackaging.m_shiny },
                { "Material", itemPackaging.m_material }
        };
    }

    template<typename BasicJsonType>
    void from_json(const BasicJsonType &j, ItemPackaging &itemPackaging)
    {
        try {
            itemPackaging.m_rigid = j["Rigid"].template get<int>();
            itemPackaging.m_shiny = j["Shiny"].template get<bool>();
            itemPackaging.m_material = j["Material"].template get<int>();
        } catch (nlohmann::json::exception &e) {
            std::cerr << "While parsing Item Packaging information, encountered exception with id: " << e.id
                      << " and the following message:\n"
                      << e.what() << '\n';
        }
    }

    template<typename BasicJsonType>
    void to_json(BasicJsonType &j, const ItemInformation& itemInfo)
    {
        j = nlohmann::json(itemInfo.m_item_dimensions);
        j.update(nlohmann::json(itemInfo.m_item_package_info), true);
    }

    template<typename BasicJsonType>
    void from_json(const BasicJsonType &j, ItemInformation &itemInfo)
    {
        itemInfo.m_item_dimensions = j.template get<ItemDimensions>();
        itemInfo.m_item_package_info = j.template get<ItemPackaging>();
    }

    template<typename BasicJsonType>
    void to_json(BasicJsonType &j, const LaneInformation& laneInfo)
    {
        j = {
                { "Num_Items", laneInfo.m_item_count },
                { "Item", laneInfo.m_item_info },
                {"Has_Tongue", laneInfo.m_has_tongue }};
        j.update(nlohmann::json(laneInfo.m_lane_index));
    }

    template<typename BasicJsonType>
    void from_json(const BasicJsonType &j, LaneInformation &laneInfo)
    {
        laneInfo.m_lane_index = j.template get<LaneIndex>();
        laneInfo.m_item_info = j["Item"].template get<ItemInformation>();
        laneInfo.m_item_count = j["Num_Items"].template get<int>();
        laneInfo.m_has_tongue = j.value("Has_Tongue", false);

    }

    template<typename BasicJsonType>
    void to_json(BasicJsonType &j, const TrayRecipe& trayRecipe)
    {
        j = {
                { "Lane_Centers", trayRecipe.m_lane_center_locs },
                { "Lane_Count", trayRecipe.m_lane_count },
                { "Max_Item_Width", trayRecipe.m_max_item_width },
                { "Min_Item_Width", trayRecipe.m_min_item_width },
                { "Name", trayRecipe.m_name }
        };
    }

    template<typename BasicJsonType>
    void from_json(const BasicJsonType &j, TrayRecipe &trayRecipe)
    {
        try {
            trayRecipe.m_lane_center_locs = j["Lane_Centers"].template get<std::vector<double>>();
            trayRecipe.m_lane_count = j["Lane_Count"].template get<int>();
            trayRecipe.m_max_item_width = j["Max_Item_Width"].template get<float>();
            trayRecipe.m_min_item_width = j["Min_Item_Width"].template get<float>();
            trayRecipe.m_name = j["Name"].template get<std::string>();
        } catch (nlohmann::json::exception &e) {
            std::cerr << "While parsing Tray Recipe information, encountered exception with id: " << e.id
                      << " and the following message:\n"
                      << e.what() << '\n';
        }
    }

} // namespace dimensional_info


namespace comms_context::identifiers {

    enum RequestType {
    FRONT_EDGE_DISTANCE = 6,
    TRAY_VALIDATION = 5,
    INVALID_REQUEST_TYPE = -1,
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(RequestType,
    {
       {INVALID_REQUEST_TYPE, "Invalid"},
       {FRONT_EDGE_DISTANCE, "FrontEdgeDistance"},
       {TRAY_VALIDATION, "TrayValidation"},
    };)

    enum CalibrationConfigKey {
    HOVER,
    TONGUE,
    DISPENSE,
    UNKNOWN = -1,
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(CalibrationConfigKey,
    {
       {UNKNOWN, "invalid_key_"},
       {DISPENSE, ""},
       {HOVER, "hover_"},
       {TONGUE, "tongue_"},
    };)

    enum TrayPositionName {
    HOVER_POSITION,
    TONGUE_POSITION,
    DISPENSE_POSITION,
    INVALID_POSITION = -1,
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(TrayPositionName,
    {
       {INVALID_POSITION, "Invalid"},
       {HOVER_POSITION, "Hover"},
       {TONGUE_POSITION, "Tongue"},
       {DISPENSE_POSITION, "Dispense"},
    };)

}

/**
 * comms_context:
 *      - OidString
 *          - Managed Mongo Object ID string
 *      - RequestContextInfo
 *          - All non-dimensional / non-recipe info in requests
 *          - Type Code, calibration mode, pkid, tray id, image prefix
 * */

namespace comms_context {
    struct OidString
    {
        std::string m_val{ json_parser::mongo_utils::make_null_id_str() };

        OidString() = default;
        explicit OidString(std::string rhs);
        explicit OidString(const nlohmann::json& rhs);
        void to_null();
        [[nodiscard]] nlohmann::json to_mongo_format() const;
        [[nodiscard]] bool is_valid() const;
        [[nodiscard]] bool is_null_id() const;
    };

    struct RequestContextInfo {
        // TODO place check on request num? Image prefix val?
        //identifiers::RequestType m_request_type{};
        //identifiers::CalibrationConfigKey m_calibration_key{};
        //identifiers::TrayPositionName m_tray_position_name{};
        int m_request_type;
        int m_calibration_mode{2};
        comms_context::OidString m_primary_key_id;
        comms_context::OidString m_tray_id;
        std::string m_image_prefix;
        std::string m_image_tag;

        std::array<const char*, 3> CalibrationKeyName{"_hover", "_tongue", ""};
        std::array<const char*, 3> TrayPositionNames {"Hover", "Tongue", "Dispense"};
        std::array<const char*, 3> RequestName {"Unknown", "TrayValidation", "FrontEdgeDistance"};
        [[nodiscard]] const char* get_calibration_mode_key() const;
        [[nodiscard]] const char* get_tray_position_name() const;
        [[nodiscard]] const char* get_request_name() const;
        [[nodiscard]] std::string get_sequence_step() const;
        [[nodiscard]] std::string get_id_tagged_sequence_step() const;
        [[nodiscard]] std::string get_primary_key_id() const;
        [[nodiscard]] nlohmann::json to_mongo_format() const;


        RequestContextInfo() = default;
        explicit RequestContextInfo(
                int requestType,
                int calibrationMode,
                const comms_context::OidString &primaryKeyId,
                const comms_context::OidString &trayId,
                const std::string &imagePrefix,
                const std::string &imageTag);


    };

    template<typename BasicJsonType>
    void to_json(BasicJsonType &j, const OidString &oidString)
    {
        j = oidString.m_val;
    }

    template<typename BasicJsonType>
    void from_json(const BasicJsonType &j, OidString &oidString)
    {
        auto get_Id = [&j]() {
            if (j.is_object() && j.template contains("$oid")) { return j.at("$oid").template get<std::string>(); }
            if (j.is_string()) { return j.template get<std::string>(); }
            return std::string(json_parser::mongo_utils::make_null_id_str());
        };
        oidString.m_val = get_Id();
        if (!oidString.is_valid()) { oidString.to_null(); }
    }

    template<typename BasicJsonType>
    void to_json(BasicJsonType &j, const RequestContextInfo &contextInfo)
    {
        j = nlohmann::json{ { "Type", contextInfo.m_request_type },
                            { "Primary_Key_ID", contextInfo.m_primary_key_id },
                            { "Tray_ID", contextInfo.m_tray_id },
                            { "Request_Name", contextInfo.get_request_name() },// TODO delete?
                            { "Calibration_Mode", contextInfo.m_calibration_mode } };
        if (!contextInfo.m_image_prefix.empty()) { j["Image_Prefix"] = contextInfo.m_image_prefix; }
        j["Image_Tag"] = contextInfo.get_id_tagged_sequence_step();//(contextInfo.m_image_tag.empty()) ? contextInfo.get_id_tagged_sequence_step() : contextInfo.m_image_tag;
    }

    template<typename BasicJsonType>
    void from_json(const BasicJsonType &j, RequestContextInfo &contextInfo)
    {
        contextInfo.m_image_prefix = j.value("Image_Prefix", "");
        contextInfo.m_request_type = j.value("Type", 0);
        contextInfo.m_calibration_mode = j.value("Calibration_Mode", 2);
        contextInfo.m_image_tag = j.value("Image_Tag", contextInfo.get_id_tagged_sequence_step());
        if (j.contains("Tray_ID")) {
            contextInfo.m_tray_id = j["Tray_ID"].template get<comms_context::OidString>(); }
        if (j.contains("Primary_Key_ID")) {
            contextInfo.m_primary_key_id = j["Primary_Key_ID"].template get<comms_context::OidString>(); }


    }

}

/**
 * request_from_vlsg:
 *      - TrayRequest
 *          - Request context info (like IDs, names, types, sequence info)
 *          - Lane info (Items in lane, count, etc)
 *          - Tray Recipe info
 * */
namespace request_from_vlsg {

    struct TrayRequest {
        // TODO place check on request num?
        comms_context::RequestContextInfo m_context {};
        std::vector<dimensional_info::LaneInformation> m_lanes {};
        dimensional_info::TrayRecipe m_tray_recipe {};
        [[nodiscard]] std::string get_primary_key_id() const;
        [[nodiscard]] std::string get_sequence_step() const;
        [[nodiscard]] float expected_max_height() const;
        TrayRequest()=default;
        TrayRequest(const comms_context::RequestContextInfo &reqContext,
                    const std::vector<dimensional_info::LaneInformation>& trayLanes,
                    dimensional_info::TrayRecipe trayRecipe);

    };

    template<typename BasicJsonType>
    void to_json(BasicJsonType &j, const TrayRequest &request)
    {
        j["Tray_Recipe"] = request.m_tray_recipe;
        j["Lanes"] = request.m_lanes;
        j.update(request.m_context); // in case j is chained.
    }

    template<typename BasicJsonType>
    void from_json(const BasicJsonType &j, TrayRequest &request)
    {
        request.m_lanes = j["Lanes"].template get<std::vector<dimensional_info::LaneInformation>>();
        // TODO Technically I don't think we need lane info for basic count
        request.m_tray_recipe = j["Tray_Recipe"].template get<dimensional_info::TrayRecipe>();
        request.m_context = j.template get<comms_context::RequestContextInfo>();
    }

} // namespace request_from_vlsg



/**
 * results_to_vlsg:
 *   Count and Item Distance results in proper format
 *      - LaneItemDistance
 *          - Returned Result from FED
 *      - BoundaryLimit
 *          - Object to handle FED max/min distance clip
 *      - LaneCounts
 *          - Counts in a single lane (a single lane elem in the result vec back to VLSg)
 *      - TrayValidationCounts
 *          - Lane Count Results and top level Errors
 *          - Will parse json the uses the TrayValidationCounts or TrayApi return format
 * */
namespace results_to_vlsg {
//TODO add Errors list obj

    struct BoundaryLimit
    {
        float m_min_distance {0};
        float m_max_distance {680};
        BoundaryLimit() = default;
        explicit BoundaryLimit(float min_distance, float max_distance);
        [[nodiscard]] float clip(float distance) const;
    };


//TODO template clip strategy -> template<typename BoundaryCheck>
    struct LaneItemDistance
    {
        dimensional_info::LaneIndex m_index {};
        std::vector<int> m_errors {0};
        float m_first_item_distance{-1};// should be float?
        float m_first_item_back_edge_distance{-1};// should be float?
        BoundaryLimit m_range_fn {};
        static nlohmann::json& to_tray_format_json(nlohmann::json& fed_res_json);
        [[nodiscard]] dimensional_info::LaneIndex get_lane_id() const;

        LaneItemDistance()=default;
        //TODO make template predicate
        explicit LaneItemDistance(dimensional_info::LaneIndex index, std::vector<int> errors, float firstItemDistance,
                                  BoundaryLimit clip);
        LaneItemDistance(int index, int error, float firstItemDistance);
        LaneItemDistance(int index, int error, float firstItemDistance, float firstItemBackEdgeDistance);
    };

    struct LaneCounts {
        dimensional_info::LaneIndex m_index{};
        std::vector<int> m_errors {{0}};
        int m_num_algorithm_counts {0};
        bool m_has_tongue{false};
        // TODO (int m_has_tongue{-1}) 0 is false, 1 is true and <0 don't report
        [[nodiscard]] dimensional_info::LaneIndex get_lane_id() const;

        LaneCounts()=default;
        explicit LaneCounts(dimensional_info::LaneIndex index,
            std::vector<int> errors,
            int laneCounts,
            bool has_tongue);
        explicit LaneCounts(int index, int error, int laneCounts, bool has_tongue);
    };

    struct TrayHeight {
      float m_max_height {-1};
      bool m_confidence {false};
      [[nodiscard]] bool confident_measurement(float max_detected_height, float max_item_height_in_tray) const;
      TrayHeight() = default;
      explicit TrayHeight(float max_detected_height, float max_item_height_in_tray);
    };

    struct TrayValidationCounts
    {
        std::vector<int> m_errors{ { 0 } };
        std::vector<results_to_vlsg::LaneCounts> m_lanes{};
        results_to_vlsg::TrayHeight m_height_info{};
        TrayValidationCounts() = default;
        TrayValidationCounts(std::vector<int> laneErrors, std::vector<results_to_vlsg::LaneCounts> algoLaneCounts);
        void update_lane_tongue_detections(std::vector<bool> tongue_detections);


    };

// TODO Fn that does merge check, merge, and recursive error clean up

    template<typename LaneResult, typename OtherLaneResult>
    constexpr bool lane_match(LaneResult lr1, OtherLaneResult lr2) {
        return ((lr1.get_lane_id() == lr2.get_lane_id()));
    }

    template<typename BasicJsonType>
    void to_json(BasicJsonType &j, const LaneItemDistance &item_distance_res)
    {
        constexpr int unknown_fed_failure_code = 5;
        j["First_Item_Distance"] = item_distance_res.m_range_fn.clip(item_distance_res.m_first_item_distance);
        j["First_Item_Back_Edge_Distance"] = item_distance_res.m_range_fn.clip(item_distance_res.m_first_item_back_edge_distance)
        j["Errors"] = (item_distance_res.m_index < 0) ? nlohmann::json::array({ unknown_fed_failure_code })
                                                      : json_parser::mongo_utils::format_obj_error_list_to_json_array(item_distance_res.m_errors);
        j.update(item_distance_res.m_index); // will there be an implicit conversion?
    }

    template<typename BasicJsonType>
    void from_json(const BasicJsonType &j, LaneItemDistance &item_distance_res)
    {
        // Most common application for this is testing parse or reading during prod app?
        item_distance_res.m_index = j.template get<dimensional_info::LaneIndex>();
        item_distance_res.m_errors = j.value("Errors", std::vector<int>{ 0 });
        item_distance_res.m_first_item_distance = item_distance_res.m_range_fn.clip(j.value("First_Item_Distance", -1));
        item_distance_res.m_first_item_back_edge_distance = item_distance_res.m_range_fn.clip(j.value("First_Item_Back_Edge_Distance", -1));
    }

    template<typename BasicJsonType>
    void to_json(BasicJsonType &j, const LaneCounts &lane_count_res)
    {
        if (lane_count_res.m_index >= 0 && lane_count_res.m_num_algorithm_counts >= 0) {
            j["Index"] = lane_count_res.m_index.m_val;
            j["Num_Algorithm_Counts"] = lane_count_res.m_num_algorithm_counts;
            j["Has_Tongue"] = lane_count_res.m_has_tongue;
        } //else { j = nlohmann::json({}); }
        j["Errors"] = json_parser::mongo_utils::format_obj_error_list_to_json_array(lane_count_res.m_errors);


    }

    template<typename BasicJsonType>
    void from_json(const BasicJsonType &j, LaneCounts &lane_count_res)
    {
        lane_count_res.m_errors = j.value("Errors", std::vector<int>{ 0 });
        lane_count_res.m_index = j.template get<dimensional_info::LaneIndex>();
        lane_count_res.m_num_algorithm_counts = j.value("Num_Algorithm_Counts", -1);

        if (lane_count_res.m_num_algorithm_counts < 0) {
            std::cerr << "\nAttempted to parse missing/invalid Num_Algorithm_Counts in LaneCounts from the "
                      << "following document:\n\t" << j << "\nCreating default m_error val!";
        }
    }

    template<typename BasicJsonType>
    void to_json(BasicJsonType &j, const TrayHeight &tray_count_res)
    {
      j = nlohmann::json{ { "MaxHeight", tray_count_res.m_max_height },
        { "MaxHeightConfidence", tray_count_res.m_confidence & (tray_count_res.m_max_height > 0)} };
    }

    template<typename BasicJsonType>
    void from_json(const BasicJsonType &j, TrayHeight &tray_count_res)
    {
      tray_count_res.m_max_height = j.value("MaxHeight", -1);
      tray_count_res.m_confidence = j.value("MaxHeightConfidence", false);
    }

    template<typename BasicJsonType>
    void to_json(BasicJsonType &j, const TrayValidationCounts &tray_count_res)
    {
        j["Errors"] = json_parser::mongo_utils::format_obj_error_list_to_json_array(tray_count_res.m_errors);
        j["Lanes"] = tray_count_res.m_lanes;
	    j["TrayHeight"] = tray_count_res.m_height_info;
    }

    template<typename BasicJsonType>
    void from_json(const BasicJsonType &j, TrayValidationCounts &tray_count_res)
    {
        tray_count_res.m_errors = j.value("Errors", std::vector<int>{ 0 });
        tray_count_res.m_height_info =  j.value("TrayHeight", TrayHeight{});
        auto contains_array_at_elem = [] (const BasicJsonType &cur_json_elem, auto key) {
            auto elem_at_key = cur_json_elem.find(key);
            if (elem_at_key == cur_json_elem.end()) { return false; }
            return elem_at_key->is_array();
        };
        auto get_lane_count_res_arr = [&j, &contains_array_at_elem]() {
            if (j.contains("count_results")) { return j.at("count_results")["Lanes"].template get<std::vector<results_to_vlsg::LaneCounts>>(); }
            if (contains_array_at_elem(j, "Lanes")) { return j["Lanes"].template get<std::vector<results_to_vlsg::LaneCounts>>(); }
            return std::vector<results_to_vlsg::LaneCounts>();
        };
        tray_count_res.m_lanes = get_lane_count_res_arr();
        if (tray_count_res.m_lanes.empty()) { tray_count_res.m_errors.emplace_back(2); }
    }

} // namespace results_to_vlsg


/**
 * tray_count_api_comms:
 *   Handles the response parse from Fulfil.TrayCountAPI, as well as request information specific to
 *   count functionality
 *      - LaneCenterLine (sent in request, returned in response)
 *          - Lane center pixel line information
 *      - BoundingBoxInfo (response only)
 *          - Coordinates and metadata info per box
 *      - DetectionAPIResults (response only)
 *          - Tray wide LaneCenterLine array, BoundingBoxInfo array
 *          - Image level info (dims, path)
 *          - Inference parameters for repeatable results (model used,
 *              image resizing, confirmation threshold)
 *          - API response status code
 * */

namespace tray_count_api_comms {
//TODO some how didn't seem to make the 'To TrayCount' Connection
    struct LaneCenterLine
    {
        // Order of numbers Front:(y, x), Back:(y,x) float b/c may be reported to img scale
        // TODO change to vec if we add more calibration pts
        float m_front_y{0.0};
        float m_front_x{0.0};
        float m_back_y{0.0};
        float m_back_x{0.0};
        LaneCenterLine() = default;
        explicit LaneCenterLine(float yFront, float xFront, float yBack, float xBack);
    };

    struct BoundingBoxInfo
    {
        dimensional_info::LaneIndex m_lane_id {};
        float m_score{0.0};
        float m_xmax {0.0};
        float m_xmin{0.0};
        float m_ymax{0.0};
        float m_ymin{0.0};
        int m_class_idx{-1};
        int m_error{0};
        BoundingBoxInfo() = default;
        explicit BoundingBoxInfo(int classIdx, dimensional_info::LaneIndex laneId,
                                 float score, float xmax, float xmin, float ymax, float ymin, int error);

    };

    struct DetectionAPIResults
    {
        std::vector<tray_count_api_comms::BoundingBoxInfo> m_bounding_boxes {};
        std::vector<tray_count_api_comms::LaneCenterLine> m_center_lines{};
        std::string m_model_version{};
        std::string m_image_file_path{};
        std::string m_used_model_path{};
        float m_shrink_factor{0.0};
        int m_input_image_height{0};
        int m_input_image_width{0};
        int m_status{500}; // 500 is server error
        [[nodiscard]] std::string extract_model_version_from_path() const;

        DetectionAPIResults()=default;
        DetectionAPIResults(std::vector<tray_count_api_comms::BoundingBoxInfo> boundingBoxes,
                            std::vector<tray_count_api_comms::LaneCenterLine> centerLines,
                            int inputImageHeight,
                            int inputImageWidth,
                            std::string modelVersion,
                            std::string imageFilePath,
                            float shrinkFactor,
                            std::string usedModelPath,
                            int  statusCode);
    };

    template<typename BasicJsonType>
    void to_json(BasicJsonType &j, const LaneCenterLine &line)
    {
        j = nlohmann::json{ { "Front", { line.m_front_y, line.m_front_x } },
                            { "Back", { line.m_back_y, line.m_back_x } } };
    }

    template<typename BasicJsonType>
    void from_json(const BasicJsonType &j, LaneCenterLine &line)
    {
        // Each pix is (y, x)
        auto center_ft = j.find("Front");
        auto center_bk = j.find("Back");
        if (center_ft != j.end() && center_bk != j.end() && center_ft->is_array() && center_bk->is_array()
            && center_ft->size() == 2 && center_bk->size() == 2) {
            line.m_front_y = center_ft.value()[0];
            line.m_front_x = center_ft.value()[1];
            line.m_back_y = center_bk.value()[0];
            line.m_back_x = center_bk.value()[1];
        }
    }

    template<typename BasicJsonType>
    void to_json(BasicJsonType &j, const BoundingBoxInfo &bounding_box_result)
    {
        j = nlohmann::json{ { "class_idx", bounding_box_result.m_class_idx },
                            { "score", bounding_box_result.m_score },
                            { "lane_id", bounding_box_result.m_lane_id },
                            { "xmax", bounding_box_result.m_xmax },
                            { "xmin", bounding_box_result.m_xmin },
                            { "ymax", bounding_box_result.m_ymax },
                            { "ymin", bounding_box_result.m_ymin },
                            { "error", int(bounding_box_result.m_lane_id == -1) } };
    }

    template<typename BasicJsonType>
    void from_json(const BasicJsonType &j, BoundingBoxInfo &bounding_box_result)
    {
        bounding_box_result.m_error = j.value("error", 0);
        try {
            j.at("class_idx").get_to(bounding_box_result.m_class_idx);
            j.at("score").get_to(bounding_box_result.m_score);
            j.at("lane_id").get_to(bounding_box_result.m_lane_id);
            j.at("xmax").get_to(bounding_box_result.m_xmax);
            j.at("xmin").get_to(bounding_box_result.m_xmin);
            j.at("ymax").get_to(bounding_box_result.m_ymax);
            j.at("ymin").get_to(bounding_box_result.m_ymin);

        } catch (nlohmann::json::out_of_range &e) {
            std::cerr << "\nAttempted to parse missing key in LaneCounts from the following document:\n\t" << j << "\n"
                      << e.what();
            std::cerr << "Creating default m_error val!";
        }
    }

    template<typename BasicJsonType>
    void to_json(BasicJsonType &j, const DetectionAPIResults &detection_results)
    {
        constexpr int min_successful_response_range = 200;
        constexpr int max_successful_response_range = 250;
        if ( detection_results.m_status >= min_successful_response_range &&
             detection_results.m_status <  max_successful_response_range) {
            j = nlohmann::json{ { "bounding_boxes", detection_results.m_bounding_boxes },
                                { "center_lines", detection_results.m_center_lines },
                                { "input_image_height", detection_results.m_input_image_height },
                                { "input_image_width", detection_results.m_input_image_width },
                                { "model_version", detection_results.m_model_version },
                                { "image_file_path", detection_results.m_image_file_path },
                                { "used_model_path", detection_results.m_used_model_path },
                                { "shrink_factor", detection_results.m_shrink_factor } };
        }
        j["status"] = detection_results.m_status;
    }

    template<typename BasicJsonType>
    void from_json(const BasicJsonType &j, DetectionAPIResults &detection_results)
    {
        detection_results.m_bounding_boxes = j.value("bounding_boxes", std::vector<tray_count_api_comms::BoundingBoxInfo>{});
        detection_results.m_center_lines = j.value("centers", std::vector<tray_count_api_comms::LaneCenterLine>{});
        detection_results.m_input_image_height = j.value("input_image_height", 0);
        detection_results.m_input_image_width = j.value("input_image_width", 0);
        detection_results.m_image_file_path = j.value("image_file_path", "");
        detection_results.m_shrink_factor = j.value("shrink_factor", float(1.0));
        detection_results.m_used_model_path = j.value("used_model_path", "");
        detection_results.m_model_version = j.value("model_version", detection_results.extract_model_version_from_path());

        constexpr int standard_tray_cnt_api_success_code = 200;
        if (!j.empty() && (detection_results.m_input_image_height + detection_results.m_input_image_width) > 0) {
            detection_results.m_status = j.value("status", standard_tray_cnt_api_success_code);
        }
    }

}


/**
 * mongo_records:
 *      - TrayCountRecord
 *          - Tray Count Result (formatted for vlsg response)
 *          - Detection results (will always show atleast the status code)
 *          - Context (put in mongo format when to_json called)
 *          - CalibrationID (for relevat tray height)
 *          - Name of VLS
 * */
namespace mongo_records {
    struct TrayCountRecord {
        results_to_vlsg::TrayValidationCounts m_tray_count{};
        tray_count_api_comms::DetectionAPIResults m_detections{};
        comms_context::RequestContextInfo m_context{};
        // From Additional m_context info
        comms_context::OidString m_calibration_id {};
        std::string m_vls_name{};
        [[nodiscard]] std::string get_primary_key_id() const;
        TrayCountRecord() = default;

        TrayCountRecord(results_to_vlsg::TrayValidationCounts trayCount,
                        tray_count_api_comms::DetectionAPIResults detectionData,
                        comms_context::RequestContextInfo context,
                        comms_context::OidString calibrationId,
                        std::string vlsName);
    };

    template<typename BasicJsonType>
    void to_json(BasicJsonType &j, const TrayCountRecord &tray_count_records)
    {
        j = tray_count_records.m_context.to_mongo_format();
        j["VlsName"] = tray_count_records.m_vls_name;
        j["status"] = tray_count_records.m_detections.m_status;
        j["count_results"] = tray_count_records.m_tray_count;
        j["CalibrationID"] = tray_count_records.m_calibration_id.to_mongo_format();

        constexpr int standard_tray_cnt_api_success_code = 200;
        if (tray_count_records.m_detections.m_status == standard_tray_cnt_api_success_code) {
            j.update(nlohmann::json(tray_count_records.m_detections));
        }
    }

    template<typename BasicJsonType>
    void from_json(const BasicJsonType &j, TrayCountRecord &tray_count_records)
    {
        tray_count_records.m_detections = j.template get<tray_count_api_comms::DetectionAPIResults>();
        tray_count_records.m_tray_count = j.template get<results_to_vlsg::TrayValidationCounts>();
        tray_count_records.m_context = j.template get<comms_context::RequestContextInfo>();
        tray_count_records.m_calibration_id = j.value("CalibrationID", comms_context::OidString());
        tray_count_records.m_vls_name = j.value("VlsName", "");
    }

}

namespace errors {
    // TODO better name
    struct FatalEdgeDistanceErrorResponse {
        std::vector<int> m_error_codes{5};
    };
    struct FatalCountErrorResponse {
        std::vector<int> m_error_codes{1};
    };

    template<typename BasicJsonType>
    void to_json(BasicJsonType &j, const FatalEdgeDistanceErrorResponse &errors_response)
    {
      constexpr float nominal_distance{-1};
      j["Errors"] = json_parser::mongo_utils::format_obj_error_list_to_json_array(errors_response.m_error_codes);
      j["First_Item_Distance"] = nominal_distance;
    }

    template<typename BasicJsonType>
    void from_json(const BasicJsonType &j, FatalEdgeDistanceErrorResponse &error_response)
    {
        error_response.m_error_codes = j.template get<std::vector<int>>();
    }

    template<typename BasicJsonType>

    void to_json(BasicJsonType &j, const FatalCountErrorResponse &error_response)
    {
        j["Errors"] = json_parser::mongo_utils::format_obj_error_list_to_json_array(error_response.m_error_codes);
        j["Lanes"] = nlohmann::json::array({});
    }

    template<typename BasicJsonType>
    void from_json(const BasicJsonType &j, FatalCountErrorResponse &error_response)
    {
        error_response.m_error_codes = j.template get<std::vector<int>>();
    }

}

#endif //FULFIL_DISPENSE_TRAY_PARSER_H
