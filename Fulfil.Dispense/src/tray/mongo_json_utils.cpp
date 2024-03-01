//
// Created by Amber Thomas on 3/9/22.
//

#include <Fulfil.Dispense/tray/mongo_json_utils.h>


nlohmann::json json_parser::mongo_utils::make_null_oid_json() {
    const char* mongo_id_specifier = "$oid";
    return nlohmann::json::object({{ mongo_id_specifier, json_parser::mongo_utils::make_null_id_str()}});
}

nlohmann::json json_parser::mongo_utils::id_str_to_mongo_oid_json(const std::string& oid_str) {
    const int oid_len = 24;
    const char* mongo_id_specifier = "$oid";
    if (oid_str.length() == oid_len) {
        return nlohmann::json::object({{mongo_id_specifier, oid_str}});
    }
    return make_null_oid_json();
}

std::string json_parser::mongo_utils::mongo_oid_json_to_id_str(const nlohmann::json& oid_json) {
    if (oid_json.is_string()) {
        return oid_json.get<std::string>();
    }
    if (oid_json.is_object() && oid_json.size() == 1 && oid_json.contains("$oid")) {
        return oid_json.at("$oid").get<std::string>();
    }
    return json_parser::mongo_utils::make_null_id_str();
}

std::string json_parser::mongo_utils::get_mongo_oid_as_id_str(const nlohmann::json& oid_json, const std::string& key) {
    auto oid_elem = oid_json.find(key);
    if (oid_elem != oid_json.end()) {
        return json_parser::mongo_utils::mongo_oid_json_to_id_str(oid_elem.value());
    }
    return json_parser::mongo_utils::make_null_id_str();

}

nlohmann::json json_parser::mongo_utils::format_obj_error_list_to_json_array(const std::vector<int> &errors) {
    if (errors.empty()) { return nlohmann::json::array({0}); }
    nlohmann::json response_errors = errors;
    if (errors.size() == 1) { return nlohmann::json(response_errors); }
    std::sort( response_errors.begin(), response_errors.end() );
    response_errors.erase( std::unique( response_errors.begin(), response_errors.end() ), response_errors.end() );
    return response_errors;
}

void json_parser::mongo_utils::format_error_json_array(nlohmann::json& errors) {
    if (!errors.is_array()) {
        std::cerr << "Issue in Errors array type! Undef behavior! Adding error code 1!";
        errors = nlohmann::json::array({1});
    }
    if (errors.size() > 1) {
        std::sort(errors.begin(), errors.end());
        auto unique_err_code_end = std::unique(errors.begin(), errors.end());
        errors.erase(unique_err_code_end, errors.end());
        if (errors[0] == 0 && errors.size() > 1)  { errors.erase(0); }

    }
    if (errors.empty()) { errors.emplace_back(0); }
}
std::string json_parser::mongo_utils::make_sequence_step_tag(const std::string &image_prefix,
                                                             const std::string &request_name,
                                                             const std::string &id_str)  {
    std::string tag = image_prefix + request_name;
    if (!id_str.empty()) {
      tag.append('_' + id_str);
    }
    return tag;
}

void json_parser::mongo_utils::merge(nlohmann::json& original_obj, nlohmann::json& update_obj) {
    auto update_elem_ref = update_obj.items().begin();
    return json_parser::mongo_utils::update(original_obj[update_elem_ref.key()],update_elem_ref.value()); // Merge objects actions
}

void json_parser::mongo_utils::update(nlohmann::json& original_obj, nlohmann::json& update_obj) {

    auto do_array_process = [](nlohmann::json& original_elem_ref_val, nlohmann::json& update_elem_ref_val) {
        return (update_elem_ref_val.is_array() && !update_elem_ref_val.empty()&& original_elem_ref_val.is_array() && !original_elem_ref_val.empty()
                && original_elem_ref_val.at(0).type_name() == update_elem_ref_val.at(0).type_name()); };
    for (auto update_elem_ref = update_obj.begin(); update_elem_ref != update_obj.end(); ++update_elem_ref)
    {
        if (update_elem_ref.value().is_structured()) {// conditions for object merging [new_val == object, iterate and merge]
            auto original_elem_ref = original_obj.find(update_elem_ref.key());
            if (original_elem_ref == original_obj.end()) {
                original_obj[update_elem_ref.key()] = update_elem_ref.value();
            } else if (update_elem_ref.value().is_object()) {// conditions for object merging [new_val == object, iterate and merge]
                json_parser::mongo_utils::update(original_obj[update_elem_ref.key()], update_elem_ref.value());
            } else if ( do_array_process(original_elem_ref.value(), update_elem_ref.value())) {
                json_parser::mongo_utils::update_array(original_obj, update_elem_ref.value(), update_elem_ref.key());
            }
        } else original_obj[update_elem_ref.key()] = update_elem_ref.value();
    }
}

void json_parser::mongo_utils::update_array(nlohmann::json& original_obj, nlohmann::json& update_elem, std::string key){
    auto original_elem = original_obj[key];
    if (original_elem.at(0).is_primitive()) {
        return json_parser::mongo_utils::array_extend(original_obj[key], update_elem);
    }
    else if (original_elem.at(0).is_structured() && original_elem.size() == update_elem.size()) {
        return json_parser::mongo_utils::array_merge(original_obj[key], update_elem);
    } else original_obj[key] = update_elem;
}

// Simple values (non structured)
void json_parser::mongo_utils::array_extend(nlohmann::json& original_elem_ref, nlohmann::json& update_elem_ref) {
    for (auto& update_val : update_elem_ref.items()) // extend the current array instead of replacing
        original_elem_ref.emplace_back(update_val.value());
}


void json_parser::mongo_utils::array_merge(nlohmann::json& original_arr, nlohmann::json& update_arr) {
    auto update_elem_ref = update_arr.begin();
    auto original_elem_ref = original_arr.begin();
    int i = 0;
    while (original_arr.end() != original_elem_ref && update_arr.end() != update_elem_ref) {
        json_parser::mongo_utils::update(original_arr[i],
                                         update_elem_ref.value()); // Rerun merge routine on the complex objects in arrays
        ++update_elem_ref; ++original_elem_ref; i++;
    }
}