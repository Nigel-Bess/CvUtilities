

#include "Fulfil.Dispense/visualization/make_media.h"
#include <Fulfil.CPPUtils/inih/INIReader.h>
#include <Fulfil.CPPUtils/inih/ini_utils.h>
#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.DepthCam/mocks.h>
#include <Fulfil.Dispense/tray/tray.h>
#include <Fulfil.Dispense/tray/tray_algorithm.h>
//#include <Fulfil.MongoCpp/mongo_connection.h>
#include <experimental/filesystem>
#include <fstream>
#include <optional>
#include <ios>

//using ff_mongo_cpp::MongoConnection;
using fulfil::utils::Logger;
using fulfil::depthcam::Session;
using fulfil::depthcam::aruco::Container;
using fulfil::depthcam::data::DataGenerator;
using fulfil::dispense::tray_processing::TrayAlgorithm;
using fulfil::dispense::tray::Tray;
namespace std_filesystem = std::experimental::filesystem;
using fulfil::utils::ini::IniSectionReader;



// Declarations
std::string TEST_START_TIME = make_media::paths::get_datetime_str();

std::optional<INIReader> parse_reader(std_filesystem::path file_name, bool use_compiled_default);


struct TrayRootData {
    std_filesystem::path m_hover_ini{};
    std_filesystem::path m_dispense_ini{};
    std_filesystem::path m_data_dir{}; // base/agx/bay/tray
    TrayRootData() = default;
    TrayRootData(std_filesystem::path hover_ini,
                 std_filesystem::path dispense_ini, std_filesystem::path data_dir);
    TrayRootData(std_filesystem::path root_dir, std_filesystem::path hover_ini,
                 std_filesystem::path dispense_ini, std_filesystem::path data_dir);
    bool is_complete();
};


struct FEDTestResultLog {
    std_filesystem::path m_result_image_path{};
    std::string m_pkid{};
    int m_front_edge_distance{-1};
    float m_execution_time_ms{0};
    bool m_has_tongue{false};
    bool m_had_test_data{false};
    FEDTestResultLog() = default;
    FEDTestResultLog(int front_edge_distance, std::string pkid, bool has_tongue,
        std_filesystem::path result_image_path, float execution_time_ms);
    friend std::ostream & operator << (std::ostream &out, const FEDTestResultLog &res_log);
};

struct TongueResultLog {
    std_filesystem::path m_result_image_path{};
    std_filesystem::path m_request_json_path{};
    std::string m_pkid{};
    float m_tongue_detection_threshold{};
    int m_lane_id{};
    bool m_tongue_mismatch{false};
    bool m_detected_tongue{false};
    bool m_req_has_tongue{false};
    double m_execution_time_ms{};
    bool m_had_test_data{false};
    TongueResultLog() = default;
    TongueResultLog(std_filesystem::path result_image_path, std::string pkid, int lane_id, bool tongue_mismatch,
        bool req_tongue, bool detected_tongue, float tongue_detection_threshold,
        double execution_time_ms, std_filesystem::path request_json_path);
    friend std::ostream & operator << (std::ostream &out, const TongueResultLog &res_log);
};

// Definitions

FEDTestResultLog::FEDTestResultLog(int front_edge_distance, std::string pkid, bool has_tongue,
    std_filesystem::path result_image_path, float execution_time_ms) :
                               m_result_image_path{result_image_path}, m_pkid{pkid}, m_front_edge_distance{front_edge_distance},
                               m_has_tongue{has_tongue}, m_execution_time_ms{execution_time_ms}, m_had_test_data{true} {}


std::ostream & operator << (std::ostream &out, const FEDTestResultLog &res_log)
{
    out << res_log.m_pkid << ',' << res_log.m_front_edge_distance << ','
      << res_log.m_has_tongue << ',' << res_log.m_execution_time_ms << ','
      << res_log.m_result_image_path;
    return out;
}

TongueResultLog::TongueResultLog(std_filesystem::path result_image_path, std::string pkid, int lane_id, bool tongue_mismatch,
    bool req_tongue, bool detected_tongue, float tongue_detection_threshold,
    double execution_time_ms, std_filesystem::path request_json_path) :
                     m_result_image_path{result_image_path}, m_request_json_path{request_json_path}, m_pkid{pkid},
                    m_tongue_detection_threshold{tongue_detection_threshold}, m_lane_id{lane_id},
                    m_tongue_mismatch{tongue_mismatch}, m_detected_tongue{detected_tongue}, m_req_has_tongue{req_tongue},
                     m_execution_time_ms{execution_time_ms}, m_had_test_data{true} {}

std::ostream & operator << (std::ostream &out, const TongueResultLog &res_log)
{
    out << res_log.m_pkid << ',' << res_log.m_lane_id << ',' << res_log.m_tongue_mismatch << ','
      << res_log.m_detected_tongue << ',' << res_log.m_req_has_tongue << ',' << res_log.m_tongue_detection_threshold
      << ',' << res_log.m_execution_time_ms << ',' << res_log.m_result_image_path << ',' << res_log.m_request_json_path;
    return out;
}

TrayRootData::TrayRootData(std_filesystem::path hover_ini, std_filesystem::path dispense_ini,
             std_filesystem::path data_dir) : m_hover_ini{hover_ini},
             m_dispense_ini{dispense_ini}, m_data_dir{data_dir} {}

TrayRootData::TrayRootData(std_filesystem::path root_dir, std_filesystem::path hover_ini,
             std_filesystem::path dispense_ini, std_filesystem::path data_dir) :
             m_hover_ini{root_dir/hover_ini}, m_dispense_ini{root_dir/dispense_ini}, m_data_dir{root_dir/data_dir} {}

bool TrayRootData::is_complete() {
    return (std_filesystem::is_regular_file(m_hover_ini)
    && std_filesystem::is_regular_file(m_dispense_ini)
    && std_filesystem::is_directory(m_data_dir));
}

std::shared_ptr<fulfil::depthcam::mocks::MockSession> make_mock_session(
        std::string directory_path, const std::string& mock_serial)
{
    if (! mock_serial.empty()){
      Logger::Instance()->Info("[test][{}] Using mock serial: {}",
          TEST_START_TIME, mock_serial);
        return std::make_unique<fulfil::depthcam::mocks::MockSession>(std::make_shared<std::string>(directory_path), mock_serial); }
    Logger::Instance()->Info("[tr] No mock serial, was supplied in test ini. To avoid manual input in the future, add to ini.");
    return std::make_unique<fulfil::depthcam::mocks::MockSession>(std::make_shared<std::string>(directory_path));
}

std::optional<INIReader> parse_reader(std_filesystem::path config_file, bool use_compiled_default) {
    auto finalized_path = [&]() {
        if (use_compiled_default) return std_filesystem::path(INIReader::get_compiled_default_dir_prefix()) / config_file;
        return config_file;
    };
    INIReader configs = INIReader(finalized_path(), false);
    if (configs.ParseError() < 0) {
        Logger::Instance()->Fatal("[test][{}] Can't load {}! Is the path correct:\n-- {}", TEST_START_TIME,
                                  config_file.filename().c_str(), finalized_path().parent_path().c_str());
        return std::nullopt;
    }
    return configs;
}

nlohmann::json parse_request_file_to_json(std_filesystem::path json_file_path) {
        // Parse request Json
    json_file_path /=  "json_request.json";
    std::ifstream json_file(json_file_path);
    nlohmann::json json_obj_data;
    json_file >> json_obj_data;
    return json_obj_data;
}

void log_tray_config_params(INIReader tray_reader, std::string vls_tray_generation) {
    auto float_value = [&] (auto field) {
        return tray_reader.GetFloat(vls_tray_generation, field);
    };
  Logger::Instance()->Info("[test][{}] Tray Config Params:\n\t* [tc] tray_config_type={}"
                   "\n\t* [tc] fiducial_width_offset={}\n\t* [tc] dispense_arm_height={}"
                    "\n\t* [tc] relative_max_height={}\n\t* [tc] relative_search_height={}\n\t* [tc] relative_min_height={}"
                    "\n\t* [tc] absolute_min_search_height_cutoff={}\n\t* [tc] relative_width_extents={}"
                    "\n\t* [tc] safe_absolute_height={}\n\t* [tc] start_deadzone={}\n\t* [tc] len_deadzone={}"
                    "\n\t* [tc] wheel_diameter_correction_mm={}\n",
                    TEST_START_TIME, vls_tray_generation,
                    float_value("fiducial_width_offset"),float_value("dispense_arm_height"),float_value("relative_max_height"),
                    float_value("relative_search_height"),float_value("relative_min_height"),
                    float_value("absolute_min_search_height_cutoff"),float_value("relative_width_extents"),
                    float_value("safe_absolute_height"),float_value("start_deadzone"),float_value("len_deadzone"),
                    float_value("wheel_diameter_correction_mm")
        );
}

FEDTestResultLog test_FED_algo(std_filesystem::path sim_data_command_dir, const std::string& mock_serial,
        std::shared_ptr<INIReader> tray_reader, std::shared_ptr<INIReader> reader)
{
    std::shared_ptr<std::string> command_id = std::make_shared<std::string>("000000000012");
    sim_data_command_dir /= "PreFrontEdgeDistance";
    if (!std_filesystem::is_directory(sim_data_command_dir)) return {};

    Logger::Instance()->Info("[test][{}] Testing against data from: {}",
                             TEST_START_TIME, std::string(sim_data_command_dir));

    auto mock_session = make_mock_session(sim_data_command_dir, mock_serial);

    // Get Mock Session
    auto request_obj = parse_request_file_to_json(sim_data_command_dir);

    auto start = std::chrono::high_resolution_clock::now();
    std::string vls_tray_generation = "tray_dimensions_" + reader->Get("device_specific", "tray_config_type", "2.1");

    request_from_vlsg::TrayRequest single_lane_val_req = request_obj.get<request_from_vlsg::TrayRequest>();
    IniSectionReader section_reader {*tray_reader, vls_tray_generation};
    TrayAlgorithm tray_algorithm = TrayAlgorithm(section_reader);
    Tray tray { single_lane_val_req.m_tray_recipe, section_reader.at<float>("tray_width"),
      section_reader.at<float>( "fiducial_width_offset") } ;
    auto [fed_result, transformed_lane_center_pixels, tongue_detections] = tray_algorithm.run_tray_algorithm(mock_session, single_lane_val_req,
                                                     tray);

    std::chrono::duration<float, std::milli> execution_time_ms = (std::chrono::high_resolution_clock::now() - start);

    std_filesystem::path result_image_path { section_reader.at<std::string>("flags", "save_data_base_path") };
    result_image_path /= "fed_results_" + make_media::paths::get_day_str();
    result_image_path /= "PreFrontEdgeDistance_" +  single_lane_val_req.get_primary_key_id() + ".jpg" ;
    return {fed_result.m_first_item_distance, single_lane_val_req.get_primary_key_id(),
                     single_lane_val_req.m_lanes[0].m_has_tongue, result_image_path, execution_time_ms.count() };
}

std::vector<TongueResultLog> test_TVR_tongue_detection(std_filesystem::path sim_data_command_dir, const std::string& mock_serial,
    std::shared_ptr<INIReader> tray_reader, std::shared_ptr<INIReader> reader)
{
    std::shared_ptr<std::string> command_id = std::make_shared<std::string>("000000000012");
    sim_data_command_dir /= "TrayValidation";
    if (!std_filesystem::is_directory(sim_data_command_dir)) return {};

    Logger::Instance()->Info("[test][{}] STARTING NEW TEST:\n\n**************\nTesting against data from: {}",
      TEST_START_TIME, std::string(sim_data_command_dir));
    // Get Mock Session
    auto mock_session = make_mock_session(sim_data_command_dir, mock_serial);
    auto request_obj = parse_request_file_to_json(sim_data_command_dir);
    auto json_file = sim_data_command_dir /  "json_request.json";

    auto start = std::chrono::high_resolution_clock::now();
    std::string vls_tray_generation = "tray_dimensions_" + reader->Get("device_specific", "tray_config_type", "2.1");
    IniSectionReader section_reader {*tray_reader, vls_tray_generation};

    auto detection_threshold = section_reader.at<float>("bead_tongue_color_limit");
    request_from_vlsg::TrayRequest tongue_det_request = request_obj.get<request_from_vlsg::TrayRequest>();
    Logger::Instance()->Info("[test][{}] Using tray dimension type: {}",
        TEST_START_TIME, vls_tray_generation);

    TrayAlgorithm tray_algorithm = TrayAlgorithm(section_reader);
    Tray tray { tongue_det_request.m_tray_recipe, section_reader.at<float>("tray_width"),
      section_reader.at<float>("fiducial_width_offset") } ;
    auto [transformed_lane_center_pixels, tongue_detections, tray_height] =
        tray_algorithm.get_pixel_lane_centers_and_tongue_detections(mock_session, tongue_det_request, tray);
    std::chrono::duration<double, std::milli> execution_time_ms = (std::chrono::high_resolution_clock::now() - start);
    auto exp_max = tongue_det_request.expected_max_height();

    Logger::Instance()->Info("[test][{}] Expected max height is {} with max height detected at {}. Expected vs. measured error: Difference = {}, Factor = {:0.4f}",
        TEST_START_TIME, exp_max, tray_height, tray_height-exp_max, tray_height/exp_max);
    if (tray_height-exp_max >= 15 or tray_height/exp_max <= 0.7){
      Logger::Instance()->Warn("[test][{}] Found suspect height differential for request {}", TEST_START_TIME, tongue_det_request.get_primary_key_id());
    }
    std::vector<TongueResultLog> tongue_detection_logs{};
    std_filesystem::path result_image_path { section_reader.at<std::string>("flags", "save_data_base_path") };
    result_image_path /= "tvr_results_" + make_media::paths::get_day_str();
    result_image_path /= tongue_det_request.m_context.get_id_tagged_sequence_step() + "_mask.jpg" ;
   std::transform(tongue_det_request.m_lanes.cbegin(), tongue_det_request.m_lanes.cend(), tongue_detections.cbegin(),
        std::back_inserter(tongue_detection_logs), [&](dimensional_info::LaneInformation req_lane, bool det_lane_tongue) {
          return TongueResultLog(result_image_path, tongue_det_request.get_primary_key_id(), req_lane.m_lane_index.m_val,
             req_lane.m_has_tongue != det_lane_tongue, req_lane.m_has_tongue, det_lane_tongue,
              detection_threshold, execution_time_ms.count(), json_file);
        }
    );
    return tongue_detection_logs;

}

// Assumes a single camera per file. Will only supply serial for first found
std::string get_serial_number_from_calibration(INIReader tray_reader){
    auto calibration_section = std::find_if(std::begin(tray_reader.Sections()), std::end(tray_reader.Sections()),
            [](std::string section) {
        std::string tray_suffix = "_tray_coordinates";
        std::string cam_suffix = "_camera_coordinates";
        return (section.length() > 28  && (std::equal(tray_suffix.rbegin(), tray_suffix.rend(), section.rbegin()) ||
            std::equal(tray_suffix.rbegin(), tray_suffix.rend(), section.rbegin())));
    });
    if (calibration_section != std::end(tray_reader.Sections())) return calibration_section->substr(0, 12);
    return"";
}

std::optional<TrayRootData> root_data_files(std_filesystem::path test_data_dir) {
    TrayRootData data_dir_info{ test_data_dir, "tray_calibration_data_hover.ini",
                                "tray_calibration_data_dispense.ini", "Tray_Camera" };
    if (data_dir_info.is_complete()) { return data_dir_info; }
    return std::nullopt;
}

std::optional<INIReader> build_tray_reader(TrayRootData sim_data) {
    auto tray_algo_config = parse_reader("tray_config.ini", true);
    auto dispense_config = parse_reader(sim_data.m_dispense_ini, false);
    auto hover_config = parse_reader(sim_data.m_hover_ini, false);

    if (tray_algo_config && dispense_config && hover_config) {
        tray_algo_config->appendReader(dispense_config.value());
        tray_algo_config->appendReader(hover_config.value());
        Logger::Instance()->Info("[test][{}] Configuration files used:\n\tDispense Calibration: {}"
                                 "\n\tHover Calibration: {}", TEST_START_TIME,
                                 std::string(sim_data.m_dispense_ini), std::string(sim_data.m_hover_ini));
        return tray_algo_config;
    }
    Logger::Instance()->Error("[test][{}] Failed to build tray reader using:"
                              "\n\tDispense Calibration: {}\n\tHover Calibration: {}",
                              TEST_START_TIME, std::string(sim_data.m_dispense_ini), std::string(sim_data.m_hover_ini));
    return std::nullopt;

}

// TestOp should hold the function call to the test to be run and the output op if any
template<typename TestOp, typename TestRecords>
TestRecords run_test_over_input_examples_directory(std_filesystem::path test_data_dir, std::shared_ptr<INIReader> reader,
                                                    TestOp run_test, TestRecords algo_results) {
    if (auto test_data_info = root_data_files(test_data_dir)) { //Base
        auto command_seqs = std_filesystem::directory_iterator{test_data_info->m_data_dir};
        if (auto tray_reader = build_tray_reader(test_data_info.value())) {
            tray_reader->appendReader(*reader);
            auto mock_serial = get_serial_number_from_calibration(tray_reader.value());
            Logger::Instance()->Info("[test][{}] Using mock serial: {}", TEST_START_TIME, mock_serial);
            std::string vls_tray_generation = "tray_dimensions_" + reader->Get("device_specific", "tray_config_type", "2.1");
            log_tray_config_params(tray_reader.value(), vls_tray_generation);
            std::transform(std_filesystem::begin(command_seqs), std_filesystem::end(command_seqs), std::back_inserter(algo_results),
                [&](auto command_dir) {
                  Logger::Instance()->Info("[test][{}] Starting test cycle {}", TEST_START_TIME);
                  return run_test(command_dir, mock_serial, std::make_shared<INIReader>(tray_reader.value()), reader); });
        }
    }
    return algo_results;
}
/*
std::shared_ptr<MongoConnection> make_test_mongo_connection() {
    auto mongo_reader = parse_reader("secret/mongo_conn_config.ini", true);
    std::string conn_str = mongo_reader->Get("connection_info", "conn_string");
    return std::make_unique<MongoConnection>(conn_str);
}
*/
std::shared_ptr<INIReader> init_main_reader(const std::string& config_section)
{
    std::string err_msg = "Test config section supplied in command line not in test ini.";
    auto main_reader = parse_reader("AGX_specific_main.ini", true);
    auto test_reader = parse_reader("test_func.ini", true);
    if (test_reader && main_reader) {
        main_reader->appendReader(test_reader.value());
        if (main_reader->Sections().find(config_section) != main_reader->Sections().end() ) {
            main_reader->set_default_section(config_section);
            return std::make_unique<INIReader>(main_reader.value());
        }
        err_msg = "Test config section supplied in command line not in test ini.";
    }
    throw std::invalid_argument(err_msg);
}



int main(int argc, char** argv) {

    std::cout << "\n\n";
    if(argc < 2) {
        throw std::runtime_error("Please specify which test config section to use from config ini on command line.");
    }
    std::string config_section {argv[1]};
    std::shared_ptr<INIReader> reader = init_main_reader(config_section);

    auto get_config = [&] (auto field, const char* default_val) {
        return bool(default_val) ? reader->Get(config_section, field, default_val) : reader->Get(config_section, field);
    };

    auto test_data_dir = get_config("test_data_dir", "");
    std::string test_type = get_config("test_type", "FED");


    Logger* test_logger = Logger::Instance((Logger::default_logging_dir +"/tray_test_logs"), "offline_tests",
            Logger::Level::TurnOff,Logger::Level::Trace);
    test_logger->SetFileLogLevel(get_config("file_log_level", "TURN_OFF"));
    test_logger->SetConsoleLogLevel(get_config("console_log_level", "TRACE"));

    test_logger->Info("[test][{}] Running new {} Tray Test:\n\tRoot Data location: {}"
                      "\n\tTest Section: {}", TEST_START_TIME, test_type, test_data_dir,
                                  reader->get_default_section());
    int record_count = 0;
    std::fstream result_csv;
    auto start_csv = [&](const std::string& result_csv_path, auto column_headers) {
        result_csv.open(result_csv_path, std::ios::out);
        result_csv << "TestTime," << column_headers << "\n";
        test_logger->Info("[test] Saving results to csv: {}", std::string(result_csv_path));
    };

    auto make_csv_path = [&](const std::string& test_type_result_name){
        std_filesystem::path result_csv_path {Logger::default_logging_dir +"/csvs/" };
        std::string res_csv_suffix = reader->Get(reader->get_default_section(), "result_log_suffix", TEST_START_TIME.substr(11));
        if (res_csv_suffix == "") { res_csv_suffix = TEST_START_TIME.substr(11); }
        if (!std_filesystem::is_directory(result_csv_path)) {
          std_filesystem::create_directories(result_csv_path);
        }
        result_csv_path /= test_type_result_name + "_" + res_csv_suffix  + ".csv";
        return result_csv_path;
    };


/************************************************************/
    if (test_type == "TVRTongue") { //run fed TRAY algo only
      std_filesystem::path result_csv_path =  make_csv_path("tongue_detection_result");

      auto write_record = [&] ( auto bay_results) {
        for (auto lane_results : bay_results) {
          for (auto result : lane_results) {
            if (result.m_had_test_data) {
              result_csv  << TEST_START_TIME<< ',' << result  << '\n';
              record_count++;
            }}
        }};

      std::string hdr_str = "Primary_Key_ID,LaneID,TongueDetectionMismatch,DetectedTongue,ExpectedTongue,DetectionThreshold,ExecTime(ms),ResultImagePath,JsonPath";
      start_csv(result_csv_path, hdr_str);
      test_logger->Info("[test][{}] Offline Tongue Detection Algo Test Using Simulated Requests in directory: {}", TEST_START_TIME, test_data_dir);
      auto run_and_log_test = [&](std_filesystem::path bay_dir) {
          std::vector<std::vector<TongueResultLog>> lane_tongue_detections{};
          auto results = run_test_over_input_examples_directory(bay_dir, reader, test_TVR_tongue_detection, lane_tongue_detections);
          if (results.empty()) { test_logger->Error("[test][{}] There was no "
              "valid data in directory:\n\t{}", TEST_START_TIME, std::string(bay_dir)); }
          write_record(results);
      };
      if(root_data_files(test_data_dir)) {
          run_and_log_test(test_data_dir);
      } else {
          auto test_bay_dirs = std_filesystem::directory_iterator{test_data_dir};
          for (auto bay : test_bay_dirs) { run_and_log_test(bay.path()); }
      }
      if (record_count == 0 ) {
          test_logger->Error("[test][{}] There were no records written to {}, deleting file!", TEST_START_TIME, std::string(result_csv_path));
          std_filesystem::remove_all(result_csv_path);
          return EXIT_FAILURE;
      }
      test_logger->Info("[test][{}] Processed and reported {} records for Tongue in lane detections!", TEST_START_TIME, record_count);

/*********************************************************************/
    } else if (test_type == "FED") {
        std_filesystem::path result_csv_path =  make_csv_path("fed_result");
        std::string hdr_str = "Primary_Key_ID,FEDTestDistance,Has_Tongue,ExecTime(ms),ResultImagePath";
        start_csv(result_csv_path, hdr_str);

        auto write_record = [&] (std::vector<FEDTestResultLog> bay_results) {
          for (auto result : bay_results) {
            if (result.m_had_test_data) {
              result_csv << TEST_START_TIME << ',' << result  << '\n';
              record_count++;
        }}};

        auto run_and_log_test = [&](std_filesystem::path bay_dir) {
          std::vector<FEDTestResultLog> algo_results{};
          auto results = run_test_over_input_examples_directory(bay_dir, reader, test_FED_algo, algo_results);
          if (results.empty()) { test_logger->Error("[test][{}] There was no valid data in directory:\n\t{}", TEST_START_TIME, std::string(bay_dir)); }
          write_record(results);
        };

        if(root_data_files(test_data_dir)) { run_and_log_test(test_data_dir); }
        else {
            auto test_bay_dirs = std_filesystem::directory_iterator{test_data_dir};
            for (auto bay : test_bay_dirs) { run_and_log_test(bay.path()); }
        }
        if (result_csv.is_open()) result_csv.close();
        if (record_count == 0 ) {
            test_logger->Error("[test][{}] There were no records written to {}, deleting file!", TEST_START_TIME, std::string(result_csv_path));
            //std_filesystem::remove_all(result_csv_path);
            return EXIT_FAILURE;
        }
        test_logger->Info("[test][{}] Processed and reported {} records for FED!", TEST_START_TIME, record_count);
   } else {
      Logger::Instance()->Error("[test] There is no tray test named ({})! Invalid option supplied by user!", test_type);
      return EXIT_FAILURE;
    }

    return 0;
}


