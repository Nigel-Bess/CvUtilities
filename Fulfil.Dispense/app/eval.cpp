

#include "Fulfil.Dispense/visualization/make_media.h"
#include <Fulfil.CPPUtils/inih/INIReader.h>
#include <Fulfil.CPPUtils/inih/ini_utils.h>
#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.DepthCam/mocks.h>
#include "Fulfil.Dispense/dispense/dispense_manager.h"
#include <Fulfil.Dispense/recipes/tray_dimensional_configuration.h>
#include <Fulfil.Dispense/tray/tray.h>
#include <Fulfil.Dispense/tray/tray_algorithm.h>
#include <Fulfil.Dispense/tray/tray_manager.h>
#include <Fulfil.Dispense/commands/item_edge_distance/item_edge_distance_response.h>
//#include <Fulfil.MongoCpp/mongo_connection.h>
#include <experimental/filesystem>
#include <fstream>
#include <optional>
#include <ios>
#include <random>

//using ff_mongo_cpp::MongoConnection;
using fulfil::dispense::commands::ItemEdgeDistanceResponse;
using fulfil::utils::Logger;
using fulfil::depthcam::Session;
using fulfil::depthcam::aruco::Container;
using fulfil::depthcam::data::DataGenerator;
using fulfil::dispense::DispenseManager;
using fulfil::dispense::tray_processing::TrayAlgorithm;
using fulfil::dispense::tray::Tray;
namespace std_filesystem = std::experimental::filesystem;
using fulfil::utils::ini::IniSectionReader;

// Declarations
std::string TEST_START_TIME = make_media::paths::get_datetime_str();

std::optional<INIReader> parse_reader(std::string file_name, bool use_compiled_default);

const int MAX_TEST_THREADS = 24;

static const std::string configs_folder = "Fulfil.Dispense/configs";

struct TrayRootData {
    std::string m_hover_ini{};
    std::string m_dispense_ini{};
    std::string m_data_dir{}; // base/agx/bay/tray
    TrayRootData() = default;
    TrayRootData(std::string hover_ini,
                 std::string dispense_ini, std::string data_dir);
    TrayRootData(std::string root_dir, std::string hover_ini,
                 std::string dispense_ini, std::string data_dir);
    bool is_complete();
};


struct FEDTestResultLog {
    std::string m_result_image_path{};
    std::string m_pkid{};
    int m_front_edge_distance{-1};
    float m_execution_time_ms{0};
    bool m_has_tongue{false};
    bool m_had_test_data{false};
    FEDTestResultLog() = default;
    FEDTestResultLog(int front_edge_distance, std::string pkid, bool has_tongue,
        std::string result_image_path, float execution_time_ms);
    friend std::ostream & operator << (std::ostream &out, const FEDTestResultLog &res_log);
};

struct TongueResultLog {
    std::string m_result_image_path{};
    std::string m_request_json_path{};
    std::string m_pkid{};
    float m_tongue_detection_threshold{};
    int m_lane_id{};
    bool m_tongue_mismatch{false};
    bool m_detected_tongue{false};
    bool m_req_has_tongue{false};
    double m_execution_time_ms{};
    bool m_had_test_data{false};
    TongueResultLog() = default;
    TongueResultLog(std::string result_image_path, std::string pkid, int lane_id, bool tongue_mismatch,
        bool req_tongue, bool detected_tongue, float tongue_detection_threshold,
        double execution_time_ms, std::string request_json_path);
    friend std::ostream & operator << (std::ostream &out, const TongueResultLog &res_log);
};

// Definitions

FEDTestResultLog::FEDTestResultLog(int front_edge_distance, std::string pkid, bool has_tongue,
    std::string result_image_path, float execution_time_ms) :
                               m_result_image_path{result_image_path}, m_pkid{pkid}, m_front_edge_distance{front_edge_distance},
                               m_has_tongue{has_tongue}, m_execution_time_ms{execution_time_ms}, m_had_test_data{true} {}


std::ostream & operator << (std::ostream &out, const FEDTestResultLog &res_log)
{
    out << res_log.m_pkid << ',' << res_log.m_front_edge_distance << ','
      << res_log.m_has_tongue << ',' << res_log.m_execution_time_ms << ','
      << res_log.m_result_image_path;
    return out;
}

std::ostream & operator << (std::ostream &out, const TongueResultLog &res_log)
{
    out << res_log.m_pkid << ',' << res_log.m_lane_id << ',' << res_log.m_tongue_mismatch << ','
      << res_log.m_detected_tongue << ',' << res_log.m_req_has_tongue << ',' << res_log.m_tongue_detection_threshold
      << ',' << res_log.m_execution_time_ms << ',' << res_log.m_result_image_path << ',' << res_log.m_request_json_path;
    return out;
}

TrayRootData::TrayRootData(std::string hover_ini, std::string dispense_ini,
             std::string data_dir) : m_hover_ini{hover_ini},
             m_dispense_ini{dispense_ini}, m_data_dir{data_dir} {}

bool TrayRootData::is_complete() {
    bool is_complete = (std_filesystem::is_regular_file(m_hover_ini)
    && std_filesystem::is_regular_file(m_dispense_ini)
    && std_filesystem::is_directory(m_data_dir));

    if (!is_complete) {
        Logger::Instance()->Info("Hov ini exists? {} {}", m_hover_ini.c_str(), std_filesystem::is_regular_file(m_hover_ini));
        Logger::Instance()->Info("dispense ini exists? {} {}", m_dispense_ini.c_str(), std_filesystem::is_regular_file(m_dispense_ini));
        Logger::Instance()->Info("data dir exists? {} {}", m_data_dir.c_str(), std_filesystem::is_directory(m_data_dir));
    }

    return is_complete;
}

// verifies that the given directory has the 3 file/folders expected (2 calibration configs & camera data folder)
std::optional<TrayRootData> root_data_files(std::string config_dir, std::string data_dir) {
    
    TrayRootData data_dir_info{ config_dir + "/tray_calibration_data_hover.ini",
                                config_dir + "/tray_calibration_data_dispense.ini",
                                data_dir };
    if (data_dir_info.is_complete()) { return data_dir_info; }
    else { throw std::invalid_argument("Invalid root_data_files"); }
    return std::nullopt;
}

std::shared_ptr<fulfil::depthcam::mocks::MockSession> make_mock_session(
        std::string directory_path, const std::string& mock_serial)
{
    if (! mock_serial.empty()){
      Logger::Instance()->Info("[test][{}] Using mock serial: {} to make mock session",
          TEST_START_TIME, mock_serial);
        return std::make_unique<fulfil::depthcam::mocks::MockSession>(std::make_shared<std::string>(directory_path), mock_serial); }
    Logger::Instance()->Info("[tr] No mock serial, was supplied in test ini. To avoid manual input in the future, add to ini.");
    return std::make_unique<fulfil::depthcam::mocks::MockSession>(std::make_shared<std::string>(directory_path));
}

std::optional<INIReader> parse_reader(std::string config_file, bool use_compiled_default) {
    auto finalized_path = [&]() {
        if (use_compiled_default) return std::string(INIReader::get_compiled_default_dir_prefix()) + config_file;
        return config_file;
    };
    INIReader configs = INIReader(finalized_path(), false);
    if (configs.ParseError() < 0) {
        Logger::Instance()->Info("[test][{}] Can't load {}! Is the path correct:\n--", TEST_START_TIME,
                                  config_file.c_str());
        return std::nullopt;
    }
    return configs;
}

nlohmann::json parse_request_file_to_json(std::string json_file_path) {
    // Parse request Json
    json_file_path +=  "/json_request.json";
    Logger::Instance()->Info("Reading: {}", json_file_path.c_str());
    std::ifstream json_file(json_file_path);
    nlohmann::json json_obj_data;
    json_file >> json_obj_data;
    return json_obj_data;
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
    Logger::Instance()->Info("Before returning serial number result");
    if (calibration_section != std::end(tray_reader.Sections())) return calibration_section->substr(0, 12);
    return"";
}

std::optional<INIReader> build_tray_reader(std::string config_dir, TrayRootData sim_data) {
    auto tray_config = config_dir + "/tray_config.ini";
    auto tray_algo_config = parse_reader(tray_config, false);
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
    Logger::Instance()->Info("Dispense calibration found at {}? {}", sim_data.m_dispense_ini.c_str(), !!dispense_config);
    Logger::Instance()->Info("Hover calibration found at {}? {}", sim_data.m_hover_ini.c_str(), !!hover_config);
    Logger::Instance()->Info("Data dir found at {}? {}", tray_config.c_str(), !!tray_algo_config);
    return std::nullopt;
}

std::shared_ptr<INIReader> init_main_reader(std::string config_file, const std::string& config_section)
{
    std::string err_msg = "Test config section supplied in command line not in test ini.";
    auto main_reader = parse_reader(config_file + "/AGX_specific_main.ini", false);
    auto test_reader = parse_reader(config_file + "/test_func.ini", false);
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

FEDTestResultLog test_FED_algo(std::string test_req_dir, std::string request_id, const std::string& mock_serial,
        std::shared_ptr<INIReader> tray_reader, std::shared_ptr<INIReader> reader)
{
    std::shared_ptr<std::string> command_id = std::make_shared<std::string>("000000000012");
    test_req_dir += "/PreFrontEdgeDistance";
    if (!std_filesystem::is_directory(test_req_dir)) {
        Logger::Instance()->Info("what dis {}", test_req_dir);
        return {};
    }

    Logger::Instance()->Info("[test][{}] Testing against data from: {}",
                             TEST_START_TIME, std::string(test_req_dir));
    auto start = std::chrono::high_resolution_clock::now();

    auto mock_session = make_mock_session(test_req_dir, mock_serial);
    auto request_obj = parse_request_file_to_json(test_req_dir);
    std::string tray_generation = "tray_dimensions_" + reader->Get("device_specific", "tray_config_type", "4.2");

    request_from_vlsg::TrayRequest single_lane_val_req = request_obj.get<request_from_vlsg::TrayRequest>();
    IniSectionReader section_reader {*tray_reader, tray_generation};
    TrayAlgorithm tray_algorithm = TrayAlgorithm(section_reader);
    Tray tray { single_lane_val_req.m_tray_recipe, section_reader.at<float>("tray_width"),
      section_reader.at<float>( "fiducial_width_offset") } ;
    auto [fed_result, transformed_lane_center_pixels, tongue_detections] = tray_algorithm.run_tray_algorithm(mock_session, single_lane_val_req, tray);

    std::chrono::duration<float, std::milli> execution_time_ms = (std::chrono::high_resolution_clock::now() - start);

    std::string result_image_path { section_reader.at<std::string>("flags", "save_data_base_path") };
    result_image_path += "fed_results_" + make_media::paths::get_day_str();
    result_image_path += "PreFrontEdgeDistance_" +  single_lane_val_req.get_primary_key_id() + ".jpg" ;

    // Dump response to JSON file
    auto fed_out = test_req_dir + "/fed_out.json";
    std::shared_ptr<nlohmann::json> fed_out_json = std::make_shared<nlohmann::json>();
    (*fed_out_json)["m_first_item_distance"] = fed_result.m_first_item_distance;
    (*fed_out_json)["px_first_item_x"] = fed_result.px_first_item_x;
    (*fed_out_json)["px_first_item_y"] = fed_result.px_first_item_y;
    (*fed_out_json)["request_id"] = request_id;

    std::ofstream out(fed_out);
    out << fed_out_json->dump();
    out.close();
    Logger::Instance()->Info("Wrote FED out: {}", fed_out);

    return {fed_result.m_first_item_distance, single_lane_val_req.get_primary_key_id(),
                     single_lane_val_req.m_lanes[0].m_has_tongue, result_image_path, execution_time_ms.count() };
}

FEDTestResultLog test_FED_dispense_manager(std::string test_req_dir, std::string request_id, const std::string& mock_serial,
        std::shared_ptr<INIReader> tray_reader, std::shared_ptr<INIReader> reader)
{
    auto start = std::chrono::high_resolution_clock::now();
    test_req_dir += "/PreFrontEdgeDistance";
    if (!std_filesystem::is_directory(test_req_dir)) {
        Logger::Instance()->Error("Invalid eval dir: {}", test_req_dir.c_str());
        throw std::invalid_argument("Invalid eval dir");
    };
    auto mock_session = make_mock_session(test_req_dir, mock_serial);
    std::string tray_generation = "tray_dimensions_" + reader->Get("device_specific", "tray_config_type", "4.2");
    fulfil::configuration::tray::TrayDimensions tray_builder =
        fulfil::configuration::tray::set_bay_wide_tray_dimensions(tray_reader, tray_generation);
    auto tray_manager = std::make_shared<fulfil::dispense::tray::TrayManager>(mock_session, tray_builder);
    DispenseManager dispense_manager(0, nullptr, mock_session, reader, tray_reader, tray_manager);
    std::shared_ptr<std::string> command_id = std::make_shared<std::string>("000000000012");

    auto request_json = std::make_shared<nlohmann::json>(parse_request_file_to_json(test_req_dir));

    std::shared_ptr<fulfil::dispense::commands::ItemEdgeDistanceResponse> response = dispense_manager.handle_item_edge_distance(command_id, request_json);

    // Dump response to JSON file
    auto fed_out = test_req_dir + "/fed_out.json";
    results_to_vlsg::LaneItemDistance item_dist = response->lane_distance_info;
    std::shared_ptr<nlohmann::json> fed_out_json = std::make_shared<nlohmann::json>();
    (*fed_out_json)["m_first_item_distance"] = item_dist.m_first_item_distance;
    (*fed_out_json)["px_first_item_x"] = item_dist.px_first_item_x;
    (*fed_out_json)["px_first_item_y"] = item_dist.px_first_item_y;
    (*fed_out_json)["request_id"] = request_id;

    std::ofstream out(fed_out);
    out << fed_out_json->dump();
    out.close();
    Logger::Instance()->Info("Wrote FED out: {}, y = {}", fed_out, item_dist.px_first_item_y);
}

std::string parseEnvVarString(std::string env_name, std::string defaultVal) {
    auto env_p = std::getenv(env_name.c_str());
    if (env_p != nullptr) {
      return env_p;
    }
    return defaultVal;
}

// Evaluate a request with the eval_type's algo, currently only "fed" supported at time of writing
void eval_request(std::string test_src, std::shared_ptr<INIReader> reader, std::string eval_type, std::string eval_request_id) {
    std::string test_instance = parseEnvVarString("TEST_INSTANCE", "test");
    // Copy all of data/by-id/<request ID>'s contents to data/test as well for future test writes
    std::string by_id_dir = "Fulfil.Dispense/" + test_src + "/Tray_Camera/" + eval_request_id;
    std::string test_dir = "Fulfil.Dispense/data/" + test_instance + "/Tray_Camera/" + eval_request_id;
    std::filesystem::create_directories(test_dir);
    // Skip existing files since it's way faster during hyperparam tuning and possibly safer
    std::filesystem::copy(by_id_dir, test_dir, std::filesystem::copy_options::skip_existing | std::filesystem::copy_options::recursive);

    Logger::Instance()->Info("Testing FED against " + test_dir);
    if(auto test_data_info = root_data_files(configs_folder, test_dir)) {
        if (auto tray_reader = build_tray_reader(configs_folder, test_data_info.value())) {
            tray_reader->appendReader(*reader);
            auto mock_serial = get_serial_number_from_calibration(tray_reader.value());
            Logger::Instance()->Info("Using mock serial: {}", mock_serial);
            // Retry each request multiple times if they fail
            const int retries = 16;
            for (int i = 0; i < retries; i++) {
                try {
                    test_FED_algo(test_dir, eval_request_id, mock_serial, std::make_shared<INIReader>(tray_reader.value()), reader);
                    break;
                } catch (const cv::Exception& e) {
                    if (i == retries - 1) {
                        throw e;
                    }
                    // Hope it's just a file data race and retry
                    // Mix in randomness to help break competing process races better
                    std::random_device rd;
                    std::mt19937 gen(rd());
                    std::uniform_int_distribution<> distrib(10, 800);
                    std::this_thread::sleep_for(std::chrono::milliseconds(distrib(gen)));
                }
            }
        }
        //test_logger->Info("Processed and reported records for FED!");
    }
}

int main(int argc, char** argv) {

    std::cout << "\n\n";
    if(argc < 2) {
        throw std::runtime_error("Please specify algo to eval");
    }
    if(argc < 3) {
        throw std::runtime_error("Please specify which request to eval");
    }
    std::string eval_type = argv[1];
    std::string eval_request_id = argv[2];

    std::shared_ptr<INIReader> reader = init_main_reader(configs_folder, "fed");
    std::string test_src = parseEnvVarString("TEST_SRC", "data/by-id");

    if (eval_request_id == "all") {
        std::vector<std::string> req_paths;
        std::vector<std::thread*> threads;
        // Grab all paths at once to avoid disk races
        for (const auto& req_path : std::filesystem::directory_iterator("Fulfil.Dispense/" + test_src + "/Tray_Camera")) {
            req_paths.push_back(req_path.path().string());
        }
        for (const auto& req_path_str : req_paths) {
            auto req_id = req_path_str.substr(req_path_str.find_last_of('/') + 1);
            Logger::Instance()->Info("Test {}", req_id);

            auto test_thread = new std::thread(eval_request, test_src, reader, eval_type, req_id);
            threads.push_back(test_thread);
            // Ensure max number of threads run but always at most X active
            if (threads.size() >= MAX_TEST_THREADS) {
                threads[0]->join();
                threads.erase(threads.begin());
            }
        }
        // Let async tests complete before exiting
        for (auto t: threads) t->join();
    } else {
        eval_request(test_src, reader, eval_type, eval_request_id);
    }
    
    //test_logger->SetFileLogLevel("TURN_OFF");
    //test_logger->SetConsoleLogLevel("TRACE");

    /*int record_count = 0;
    std::fstream result_csv;
    auto start_csv = [&](const std::string& result_csv_path, auto column_headers) {
        result_csv.open(result_csv_path, std::ios::out);
        result_csv << "TestTime," << column_headers << "\n";
        test_logger->Info("[test] Saving results to csv: {}", std::string(result_csv_path));
    };

    auto make_csv_path = [&](const std::string& test_type_result_name){
        std::string result_csv_path {Logger::default_logging_dir +"/csvs/" };
        std::string res_csv_suffix = reader->Get(reader->get_default_section(), "result_log_suffix", TEST_START_TIME.substr(11));
        if (res_csv_suffix == "") { res_csv_suffix = TEST_START_TIME.substr(11); }
        if (!std_filesystem::is_directory(result_csv_path)) {
          std_filesystem::create_directories(result_csv_path);
        }
        result_csv_path += test_type_result_name + "_" + res_csv_suffix  + ".csv";
        return result_csv_path;
    };
    */


/************************************************************/
        /*std::string result_csv_path =  make_csv_path("fed_result");
        std::string hdr_str = "Primary_Key_ID,FEDTestDistance,Has_Tongue,ExecTime(ms),ResultImagePath";
        start_csv(result_csv_path, hdr_str);

        auto write_record = [&] (std::vector<FEDTestResultLog> bay_results) {
          for (auto result : bay_results) {
            if (result.m_had_test_data) {
              result_csv << TEST_START_TIME << ',' << result  << '\n';
              record_count++;
        }}};*/
    std::exit(0);
    return 0;
}
