//
// Created by amber on 4/1/22.
//

#ifndef FULFIL_DISPENSE_MONGO_JSON_UTILS_H
#define FULFIL_DISPENSE_MONGO_JSON_UTILS_H

#include <iostream>
#include <json.hpp>


namespace json_parser::mongo_utils {

        constexpr const char* make_null_id_str() {  return "000000000000000000000000"; }

        nlohmann::json make_null_oid_json();

        nlohmann::json id_str_to_mongo_oid_json(const std::string& oid_str);

        std::string mongo_oid_json_to_id_str(const nlohmann::json& oid_json);

        std::string get_mongo_oid_as_id_str(const nlohmann::json& oid_json, const std::string& key);

        nlohmann::json format_obj_error_list_to_json_array(const std::vector<int> &errors);

        void format_error_json_array(nlohmann::json& errors);

        std::string make_sequence_step_tag(const std::string &image_prefix,
                                           const std::string &request_name,
                                           const std::string &id_str);

        void array_merge(nlohmann::json& original_arr, nlohmann::json& update_arr);

        void merge(nlohmann::json& update_elem_ref, nlohmann::json& original_obj);

        void array_extend(nlohmann::json& original_elem_ref, nlohmann::json& update_elem_ref);

        void update(nlohmann::json& original_obj, nlohmann::json& update_obj);

        void update_array(nlohmann::json& original_elem_ref, nlohmann::json& update_elem_ref, std::string key);

}

#endif //FULFIL_DISPENSE_MONGO_JSON_UTILS_H
