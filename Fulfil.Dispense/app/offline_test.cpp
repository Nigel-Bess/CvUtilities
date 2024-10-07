#include<Fulfil.DepthCam/core.h>
#include<Fulfil.DepthCam/visualization.h>
#include<Fulfil.DepthCam/point_cloud.h>
#include<Fulfil.DepthCam/mocks.h>
#include<Fulfil.DepthCam/data.h>
#include<opencv2/opencv.hpp>
#include "Fulfil.Dispense/dispense/dispense_manager.h"
#include <Fulfil.Dispense/commands/parsing/dispense_json_parser.h>
#include <Fulfil.Dispense/commands/parsing/dispense_request_parser.h>
#include <Fulfil.Dispense/commands/error_response.h>
#include <Fulfil.Dispense/dispense/dispense_processing_queue_predicate.h>
#include <dirent.h>
#include <Fulfil.DepthCam/data/null_sender.h>
#include <Fulfil.CPPUtils/file_system_util.h>
#include <Fulfil.CPPUtils/inih/INIReader.h>
#include <FulfilMongoCpp/mongo_connection.h>
#include <Fulfil.Dispense/version.h>


using ff_mongo_cpp::MongoConnection;
using fulfil::utils::Logger;
using fulfil::configuration::lfb::LfbVisionConfiguration;
using fulfil::dispense::DispenseManager;
using fulfil::dispense::commands::DispenseResponse;
using fulfil::dispense::commands::DispenseRequest;
using fulfil::depthcam::Session;
using fulfil::depthcam::pointcloud::PointCloud;
using fulfil::dispense::drop::DropResult;
using fulfil::dispense::commands::DropTargetDetails;
using fulfil::depthcam::aruco::MarkerDetector;
using fulfil::depthcam::aruco::Container;
using fulfil::depthcam::aruco::MarkerDetectorContainer;
using fulfil::depthcam::data::DataGenerator;
using fulfil::dispense::drop::DropManager;
using fulfil::dispense::commands::DispenseRequestParser;
using fulfil::dispense::commands::DispenseJsonParser;
using fulfil::utils::processingqueue::ProcessingQueue;
using fulfil::dispense::commands::ErrorResponse;
using fulfil::utils::processingqueue::ProcessingQueuePredicate;
using fulfil::utils::FileSystemUtil;
using fulfil::dispense::visualization::LiveViewer;
using fulfil::depthcam::data::NullSender;
using fulfil::depthcam::data::NullSender;
using fulfil::dispense::commands::PostLFRResponse;


float to_meters(float millimeters) {
	return (millimeters) / 1000.F;
}


void start_mock_session(std::shared_ptr<std::string> directory_path,
						std::shared_ptr<fulfil::depthcam::mocks::MockSession> &mock_session,
						const std::string &mock_serial) {
	if (mock_serial == "NOT USED") {
		Logger::Instance()->Info(
				"No mock serial, was supplied in test ini. To avoid manual input in the future, add to ini.");
		mock_session = std::make_shared<fulfil::depthcam::mocks::MockSession>(directory_path);
	} else {
		mock_session = std::make_shared<fulfil::depthcam::mocks::MockSession>(directory_path, mock_serial);
	}
}

/**
 * Reads the JSON found at the given location into a json obj
 */
std::shared_ptr<nlohmann::json> read_in_request_json(std::string directory_path,
													 std::string json_file_name) {
	Logger::Instance()->Debug("Reading json request from file now");
	std::string file_path = make_media::paths::join_as_path(directory_path, json_file_name);
	Logger::Instance()->Debug("JSON File location is: {}", file_path);
	std::ifstream ifs(file_path);
	std::shared_ptr<nlohmann::json> json_contents = std::make_shared<nlohmann::json>(nlohmann::json::parse(ifs));
	Logger::Instance()->Debug("JSON contents is: {}", json_contents->dump());
	return json_contents;
}

void test_pre_drop_routine_simulated(std::shared_ptr<std::string> directory_path, std::vector<int> test_items,
									 std::shared_ptr<INIReader> reader) {
	std::shared_ptr<fulfil::depthcam::mocks::MockSession> mock_session;
	std::string mock_serial = reader->Get(reader->get_default_section(), "mock_serial", "NOT USED");
	start_mock_session(directory_path, mock_session, mock_serial);
	if (mock_session == nullptr) Logger::Instance()->Fatal("The mock session was improperly initialized!!!");

	//NOTE that width and length of container are given in meters

	// Creating the DropManager
	std::shared_ptr<fulfil::dispense::drop::DropManager> manager = std::make_shared<fulfil::dispense::drop::DropManager>(
			mock_session, reader, nullptr);

	// Creating the requests
	std::shared_ptr<std::string> request_id = std::make_shared<std::string>("000000000012");

//  std::ifstream ifs("LFB3_bag_state_default.json");
//  std::string content( (std::istreambuf_iterator<char>(ifs) ),
//                       (std::istreambuf_iterator<char>()    ) );
//  std::string bag_state_json_str = "LFB3_bag_state_default.json";
//  nlohmann::json bag_state_json = nlohmann::json::parse(bag_state_json_str);
//  Logger::Instance()->Info("Bag state JSON: ", bag_state_json["LfbConfig"]);
//  auto cvbag = fulfil::mongo::CvBagState(bag_state_json);
//  manager->mongo_bag_state->parse_in_values((std::make_shared<fulfil::mongo::CvBagState>(cvbag)));

	std::vector<std::shared_ptr<fulfil::dispense::commands::DropTargetDetails>> requests;
	// id numbers for request on right

	for (auto it = test_items.begin(); it != test_items.end(); ++it) {
		std::cout << std::endl;
		Logger::Instance()->Info("Handling request #{}", *it);
		/**
		*  Reading .json request from file
		*/
		std::shared_ptr<nlohmann::json> request_json = read_in_request_json(*directory_path, "json_request.json");
		requests.push_back(std::make_shared<DropTargetDetails>(request_json, request_id));

//    Logger::Instance()->Debug("Reading json request from file now");
//    std::shared_ptr<std::string> file_path = std::make_shared<std::string>();  //read json from file
//    file_path->append(*directory_path);
//    *file_path = make_media::paths::join_as_path(*file_path, "json_request.json");
//    Logger::Instance()->Debug("File location is: {}", *file_path);
//    std::ifstream ifs( *file_path);
//    std::string req_content((std::istreambuf_iterator<char>(ifs) ),
//                        (std::istreambuf_iterator<char>()));
//    Logger::Instance()->Debug("Request JSON content is: {}", req_content);
//    std::shared_ptr<nlohmann::json> request_json = std::make_shared<nlohmann::json>(nlohmann::json::parse(req_content.c_str()));

//      std::ifstream ifs("LFB3_bag_state_default.json");

		std::shared_ptr<nlohmann::json> bag_state_json = read_in_request_json(*directory_path, "bag_state.json");
//    std::shared_ptr<std::string> bag_state_file_path = std::make_shared<std::string>();  //read json from file
//    bag_state_file_path->append(*directory_path);
//    *bag_state_file_path = make_media::paths::join_as_path(*bag_state_file_path, "bag_state.json");
////      /home/jessv/code/Fulfil.Dispense/data/apr04/660eee5f027cca9aea0c17e8/Drop_Target_Image/2024_04_04_H11_M16_S00
//    Logger::Instance()->Debug("File location is: {}", *bag_state_file_path);
//    std::ifstream bag( *bag_state_file_path);
//    std::string content( (std::istreambuf_iterator<char>(bag) ),
//                         (std::istreambuf_iterator<char>()    ) );
////      std::string bag_state_json_str = "LFB3_bag_state_default.json";
//    Logger::Instance()->Info("Bag state JSON: ", content);

//    nlohmann::json bag_state_json = nlohmann::json::parse(content.c_str());
//      nlohmann::json empty = nlohmann::json({});

//      *bag_state_json = bag_state_json->value("Bag_State", empty);
		//    Logger::Instance()->Info("Bag state JSON: ", content);
//      Logger::Instance()->Debug("JSON contents is: {}", bag_state_json->dump());


		Logger::Instance()->Debug("BagId is: {}", bag_state_json->value("BagId", "NONE"));
		Logger::Instance()->Debug("BagId string is: {}", (*bag_state_json)["BagId"].get<std::string>());
//    Logger::Instance()->Debug("MongoID string is: {}", (*bag_state_json)["MongoID"].get<std::string>());
//    (*bag_state_json)["LfbConfig"] = empty;
//    Logger::Instance()->Debug("LfbConfig string is: {}", std::string((*bag_state_json)));

		auto cvbag = std::make_shared<fulfil::mongo::CvBagState>(*bag_state_json);
		// read in default lfb vision config json
		// pass in!
		/**
		 * LFB Vision config json default :cry: but also is the default file even needed bc tecnically it should work without any file
		 */
//      std::string prefix = DEFAULT_FULFIL_INI_DIR;
//      if (prefix.back() != '/') prefix.append("/");
//      filename = (filename.front() == '/') ? prefix.append(&filename[1]) : prefix.append(filename);
//    cvbag->Config =
		manager->mongo_bag_state->parse_in_values(cvbag);
//            std::make_shared<fulfil::mongo::CvBagState>(cvbag)));

		manager->handle_drop_request(request_json,
									 requests.at(*it),
									 directory_path, FileSystemUtil::create_datetime_string(),
									 false); // parameters
	}
	Logger::Instance()->Info("Successfully completed routine");
}


void test_pre_drop_routine_json(std::shared_ptr<std::string> directory_path, std::shared_ptr<INIReader> reader) {
	std::shared_ptr<fulfil::depthcam::mocks::MockSession> mock_session;
	std::shared_ptr<fulfil::depthcam::mocks::MockSession> mock_session_tray;
	std::string mock_serial = reader->Get(reader->get_default_section(), "mock_serial", "NOT USED");
	start_mock_session(directory_path, mock_session, mock_serial);
	if (mock_session == nullptr) Logger::Instance()->Fatal("The mock session was improperly initialized!!!");

	// Creating the DropManager
	std::shared_ptr<fulfil::dispense::drop::DropManager> manager = std::make_shared<fulfil::dispense::drop::DropManager>(
			mock_session, reader, nullptr);

	std::shared_ptr<nlohmann::json> request_json = read_in_request_json(*directory_path, "json_request.json");
	std::shared_ptr<std::string> request_id = std::make_shared<std::string>("000000000012");

	std::shared_ptr<fulfil::dispense::commands::DropTargetDetails> offline_drop_details =
			std::make_shared<fulfil::dispense::commands::DropTargetDetails>(request_json, request_id);

	std::cout << std::endl;
	Logger::Instance()->Info("Handling request now");
	manager->handle_drop_request(request_json, offline_drop_details, directory_path,
								 FileSystemUtil::create_datetime_string(), false); // parameters
	Logger::Instance()->Info("Successfully completed routine");
}


void test_post_drop_routine(std::shared_ptr<std::string> base_directory, std::shared_ptr<std::string> directory_path,
							std::shared_ptr<INIReader> reader) {
	std::shared_ptr<fulfil::depthcam::mocks::MockSession> mock_session;
	std::string mock_serial = reader->Get(reader->get_default_section(), "mock_serial", "NOT USED");
	start_mock_session(directory_path, mock_session, mock_serial);
	if (mock_session == nullptr) Logger::Instance()->Fatal("The mock session was improperly initialized!!!");

	// Creating the DropManager
	std::shared_ptr<fulfil::dispense::drop::DropManager>
			manager = std::make_shared<fulfil::dispense::drop::DropManager>(mock_session, reader, nullptr);

	std::shared_ptr<LfbVisionConfiguration> lfb_vision_config = std::make_shared<LfbVisionConfiguration>();

	std::shared_ptr<std::string> request_id = std::make_shared<std::string>("000000000012");
	bool extend_depth_analysis_over_markers = lfb_vision_config->extend_depth_analysis_over_markers;

	std::shared_ptr<MarkerDetectorContainer> container = manager->searcher->get_container(lfb_vision_config,
																						  mock_session,
																						  extend_depth_analysis_over_markers);


	Logger::Instance()->Debug("Reading post json request from file now");
	*directory_path = make_media::paths::join_as_path(*directory_path, "json_request.json");
	Logger::Instance()->Info("Reading json from file location is: {}", *directory_path);
	std::ifstream ifs2(*directory_path);
	std::shared_ptr<nlohmann::json> post_request_json = std::make_shared<nlohmann::json>(nlohmann::json::parse(ifs2));

	ff_mongo_cpp::mongo_objects::MongoObjectID bag_oid = ff_mongo_cpp::mongo_objects::MongoObjectID(
			"5e6fe411b901a80c5481d4e5");
	std::shared_ptr<fulfil::mongo::MongoBagState> doc = std::make_shared<fulfil::mongo::MongoBagState>(bag_oid, false);
//  manager->handle_post_LFR(post_request_json, base_directory, request_id, false);
	manager->searcher->find_max_Z(container, request_id, lfb_vision_config, doc, post_request_json, nullptr,
								  base_directory); // parameters


	/**
	 * Add additional check for products fitting in bag
	 */
//   manager->cached_post_container = container;
//   std::vector<int> products_to_overflow = manager->check_products_for_fit_in_bag(LFB_config_reader, post_request_json);
//   std::cout << "offline test for product fit included " << products_to_overflow.size() << " products" << std::endl;
//
//   PostLFRResponse test_response = PostLFRResponse(request_id, 0);
//   test_response.set_products_to_overflow(products_to_overflow);
//   test_response.dispense_payload();
}

int
test_compare_pre_post(std::shared_ptr<std::string> directory_path_pre, std::shared_ptr<std::string> directory_path_post,
					  std::shared_ptr<std::string> directory_path_target, std::shared_ptr<INIReader> reader) {
	Logger::Instance()->Fatal("This test is not currently supported!");
//    //TODO: needs many modifications
//  std::shared_ptr<fulfil::depthcam::mocks::MockSession> mock_session_pre;
//  std::shared_ptr<fulfil::depthcam::mocks::MockSession> mock_session_post;
//  std::string mock_serial = reader->Get(reader->get_default_section(), "mock_serial", "NOT USED");
//  if (mock_serial == "NOT USED")
//  {
//    Logger::Instance()->Info(
//        "No mock serial, was supplied in test ini. To avoid manual input in the future, add to ini.");
//    mock_session_pre = std::make_shared<fulfil::depthcam::mocks::MockSession>(directory_path_pre);
//    mock_session_post = std::make_shared<fulfil::depthcam::mocks::MockSession>(directory_path_post);
//  }
//  else
//  {
//    mock_session_pre = std::make_shared<fulfil::depthcam::mocks::MockSession>(directory_path_pre, mock_serial);
//    mock_session_post = std::make_shared<fulfil::depthcam::mocks::MockSession>(directory_path_post, mock_serial);
//  }
//
//  //load request_json files from pre and post saved data //TODO: create a separate function for reading json files from directory and storing in request_json pointer. Used above as well
//  Logger::Instance()->Debug("Reading pre json request from file now");
//  std::shared_ptr<std::string> file_path = std::make_shared<std::string>();  //read json from file
//  file_path->append(*directory_path_pre);
//  *file_path = make_media::paths::join_as_path(*file_path, "json_request.json");
//  Logger::Instance()->Debug("File location is: {}", *file_path);
//  std::ifstream ifs( *file_path);
//  std::shared_ptr<nlohmann::json> pre_request_json = std::make_shared<nlohmann::json>(nlohmann::json::parse(ifs));
//  Logger::Instance()->Debug("PRE JSON REQUEST:{}", pre_request_json->dump());
//
//  Logger::Instance()->Debug("Reading post json request from file now");
//  file_path = std::make_shared<std::string>();  //read json from file
//  file_path->append(*directory_path_post);
//  *file_path = make_media::paths::join_as_path(*file_path, "json_request.json");
//  Logger::Instance()->Debug("File location is: {}", *file_path);
//  std::ifstream ifs2( *file_path);
//  std::shared_ptr<nlohmann::json> post_request_json = std::make_shared<nlohmann::json>(nlohmann::json::parse(ifs2));
//  Logger::Instance()->Debug("POST JSON REQUEST:{}", post_request_json->dump());
//
//  Logger::Instance()->Debug("Reading drop target json request from file now");
//  file_path = std::make_shared<std::string>();  //read json from file
//  file_path->append(*directory_path_target);
//  *file_path = make_media::paths::join_as_path(*file_path, "json_request.json");
//  Logger::Instance()->Debug("File location is: {}", *file_path);
//  std::ifstream ifs3( *file_path);
//  std::shared_ptr<nlohmann::json> drop_target_json = std::make_shared<nlohmann::json>(nlohmann::json::parse(ifs3));
//  Logger::Instance()->Debug("DROP TARGET JSON REQUEST:{}", drop_target_json->dump());
//
//
//  Logger::Instance()->Debug("Reading error code and (if available) target X and Y values now");
//  file_path = std::make_shared<std::string>();  //read json from file
//  file_path->append(*directory_path_pre);
//  *file_path = make_media::paths::join_as_path(*file_path, "error_code");
//  Logger::Instance()->Debug("File location is: {}", *file_path);
//  std::ifstream ifs_error( *file_path);
//  std::string s;
//  std::getline(ifs_error, s);
//
//  //if error code = 0, then read in the given target info. Otherwise pass in values of -1 as invalid target data
//  int error_code = std::stoi(s);
//  float target_x;
//  float target_y;
//
//  if (error_code != 0)
//  {
//    target_x = -1;
//    target_y = -1;
//  }
//  else
//  {
//    file_path = std::make_shared<std::string>();  //read json from file
//    file_path->append(*directory_path_target);
//    *file_path = make_media::paths::join_as_path(*file_path, "target_center");
//    Logger::Instance()->Debug("Target file location is: {}", *file_path);
//    std::ifstream ifs_target( *file_path);
//    std::string target_x_line;
//    std::string target_y_line;
//    std::getline(ifs_target, target_x_line);
//    std::getline(ifs_target, target_y_line);
//    target_x = std::stof(target_x_line) / 1000; //target coordinates converted to meters
//    target_y = std::stof(target_y_line) / 1000;
//  }
//
//  Logger::Instance()->Debug("Read target data as X: {}, Y: {}", target_x, target_y);
//
//  Logger::Instance()->Trace("Creating DropManager from mock_session_post now");
//  // Creating the DropManager
//  std::shared_ptr<fulfil::dispense::drop::DropManager>
//      manager = std::make_shared<fulfil::dispense::drop::DropManager>(mock_session_post, reader, nullptr);
//
//  std::shared_ptr<std::string> request_id = std::make_shared<std::string>("000000000012");
//  Logger::Instance()->Trace("Calling dropzonesearcher compare_pre_post now");
//
//  bool extend_depth_analysis_over_markers = LFB_config_reader->GetBoolean("LFB_config", "extend_depth_analysis_over_markers", false);
//
//  std::shared_ptr<MarkerDetectorContainer> pre_container = manager->searcher->get_container(LFB_config_reader, mock_session_pre, extend_depth_analysis_over_markers);
//  std::shared_ptr<MarkerDetectorContainer> post_container = manager->searcher->get_container(LFB_config_reader, mock_session_post, extend_depth_analysis_over_markers);
//
//  std::shared_ptr<cv::Mat> result_mat = nullptr;
//  int target_item_overlap = 0;
//  int result = manager->pre_post_compare->run_comparison(pre_container, post_container, LFB_config_reader, pre_request_json,
//                                                         post_request_json, drop_target_json, cv::Point2f(target_x,target_y),
//                                                         &result_mat, &target_item_overlap); // parameters
//  Logger::Instance()->Debug("Percentage of item that landed on target is: {}", target_item_overlap);
//
//  if(result == 0)
//  {
//    std::cout << "No items were detected" << std::endl;
//  }
//  else
//  {
//    std::cout << result << std::endl;
//  }
//
//  cv::waitKey(0);
//  cv::destroyAllWindows();
//
//  Logger::Instance()->Debug("Pre Post Compare finished in offline_test");

//  return result;
	return 0;
//  Logger::Instance()->Debug("Reading post json request from file now");
//  std::shared_ptr<nlohmann::json> post_request_json = read_in_request_json(*directory_path_post, "json_request.json");
//  Logger::Instance()->Debug("POST JSON REQUEST:{}", post_request_json->dump());
//
//  Logger::Instance()->Debug("Reading drop target json request from file now");
//  std::shared_ptr<nlohmann::json> drop_target_json = read_in_request_json(*directory_path_target, "json_request.json");
//  Logger::Instance()->Debug("DROP TARGET JSON REQUEST:{}", drop_target_json->dump());
//
//
//  Logger::Instance()->Debug("Reading error code and (if available) target X and Y values now");
//  std::shared_ptr<std::string> file_path = std::make_shared<std::string>();  //read json from file
//  file_path->append(*directory_path_pre);
//  *file_path = make_media::paths::join_as_path(*file_path, "error_code");
//  Logger::Instance()->Debug("File location is: {}", *file_path);
//  std::ifstream ifs_error( *file_path);
//  std::string s;
//  std::getline(ifs_error, s);
//
//  //if error code = 0, then read in the given target info. Otherwise pass in values of -1 as invalid target data
//  int error_code = std::stoi(s);
//  float target_x;
//  float target_y;
//
//  if (error_code != 0)
//  {
//    target_x = -1;
//    target_y = -1;
//  }
//  else
//  {
//    file_path = std::make_shared<std::string>();  //read json from file
//    file_path->append(*directory_path_target);
//    *file_path = make_media::paths::join_as_path(*file_path, "target_center");
//    Logger::Instance()->Debug("Target file location is: {}", *file_path);
//    std::ifstream ifs_target( *file_path);
//    std::string target_x_line;
//    std::string target_y_line;
//    std::getline(ifs_target, target_x_line);
//    std::getline(ifs_target, target_y_line);
//    target_x = std::stof(target_x_line) / 1000; //target coordinates converted to meters
//    target_y = std::stof(target_y_line) / 1000;
//  }
//
//  Logger::Instance()->Debug("Read target data as X: {}, Y: {}", target_x, target_y);
//
//  Logger::Instance()->Trace("Creating DropManager from mock_session_post now");
//  // Creating the DropManager
//  std::shared_ptr<fulfil::dispense::drop::DropManager>
//      manager = std::make_shared<fulfil::dispense::drop::DropManager>(mock_session_post, reader, nullptr);
//
//  std::shared_ptr<std::string> request_id = std::make_shared<std::string>("000000000012");
//  Logger::Instance()->Trace("Calling dropzonesearcher compare_pre_post now");
//
//  bool extend_depth_analysis_over_markers = LFB_config_reader->GetBoolean("LFB_config", "extend_depth_analysis_over_markers", false);
//
//  std::shared_ptr<MarkerDetectorContainer> pre_container = manager->searcher->get_container(LFB_config_reader, mock_session_pre, extend_depth_analysis_over_markers);
//  std::shared_ptr<MarkerDetectorContainer> post_container = manager->searcher->get_container(LFB_config_reader, mock_session_post, extend_depth_analysis_over_markers);
//
//  std::shared_ptr<cv::Mat> result_mat = nullptr;
//  int target_item_overlap = 0;
//  int result = manager->pre_post_compare->run_comparison(pre_container, post_container, LFB_config_reader, pre_request_json,
//                                                         post_request_json, drop_target_json, cv::Point2f(target_x,target_y),
//                                                         &result_mat, &target_item_overlap); // parameters
//  Logger::Instance()->Debug("Percentage of item that landed on target is: {}", target_item_overlap);
//
//  if(result == 0)
//  {
//    std::cout << "No items were detected" << std::endl;
//  }
//  else
//  {
//    std::cout << result << std::endl;
//  }
//
//
//  cv::waitKey(0);
//  cv::destroyAllWindows();
//
//  Logger::Instance()->Debug("Pre Post Compare finished in offline_test");
//
//  return result;
}

void test_item_on_ground_post_drop(std::shared_ptr<std::string> directory_path, std::shared_ptr<INIReader> reader) {
	std::shared_ptr<fulfil::depthcam::mocks::MockSession> mock_session;
	std::string mock_serial = reader->Get(reader->get_default_section(), "mock_serial", "NOT USED");
	start_mock_session(directory_path, mock_session, mock_serial);
	if (mock_session == nullptr) Logger::Instance()->Fatal("The mock session was improperly initialized!!!");

	// Creating the DropManager
	std::shared_ptr<fulfil::dispense::drop::DropManager> manager = std::make_shared<fulfil::dispense::drop::DropManager>(
			mock_session, reader, nullptr);

	std::shared_ptr<nlohmann::json> request_json = read_in_request_json(*directory_path, "json_request.json");
	std::shared_ptr<std::string> request_id = std::make_shared<std::string>("000000000012");

	std::shared_ptr<fulfil::dispense::commands::DropTargetDetails> offline_drop_details =
			std::make_shared<fulfil::dispense::commands::DropTargetDetails>(request_json, request_id);

	std::cout << std::endl;
	Logger::Instance()->Info("Handling request now");
	manager->handle_post_LFR(request_json, directory_path, FileSystemUtil::create_datetime_string(),
							 false); // parameters
	Logger::Instance()->Info("Successfully completed routine");
}


int main(int argc, char **argv) {

	/**
	 * Basic test set-up
	 * */
	if (argc < 2) {
		throw std::runtime_error("Please specify which test config section to use from config ini on command line.");
	}

	Logger *test_logger = Logger::Instance((Logger::default_logging_dir + "/test_logs"), "offline_tests",
										   Logger::Level::TurnOff, Logger::Level::Trace);


	Logger::Instance()->Debug("Latest FW Tag found: {}. Build generated on {}.\nDispense API commit details: {} ({})",
							  FW_VERSION, BUILD_DATE, DISPENSE_COMMIT, IS_SOURCE_REPO_CLEAN);

	std::string config_section = argv[1];
	Logger::Instance()->Fatal("Offline test called with test section: {}", config_section);


	std::shared_ptr<INIReader> reader = std::make_shared<INIReader>("test_func.ini", true);
	reader->appendReader(INIReader("AGX_specific_main.ini", true));

	if (reader->ParseError() < 0) {
		test_logger->Fatal("Can't load test_func.ini, check path.");
		throw std::runtime_error("Failure to parse ini file...");
	}

	std::string console_log_level = reader->Get(config_section, "console_log_level", "TRACE");
	std::string file_log_level = reader->Get(config_section, "file_log_level", "TURN_OFF");
	test_logger->SetConsoleLogLevel(console_log_level);
	test_logger->SetFileLogLevel(file_log_level);

	if (reader->Sections().find(config_section) == reader->Sections().end()) {
		test_logger->Fatal("The specified config section is not in test_func.ini ");
		throw std::invalid_argument("Test config section supplied in command line not in test ini.");
	}

	// Get mongo configs
	std::shared_ptr<INIReader> mongo_reader = std::make_shared<INIReader>("../secret/mongo_conn_config.ini", true);
	if (mongo_reader->ParseError() < 0) {
		Logger::Instance()->Fatal("Failure to load mongo credentials, check path.");
		throw std::runtime_error("Failure to parse ini file...");
	}
	std::string conn_str = mongo_reader->Get("connection_info", "conn_string");
	std::shared_ptr<MongoConnection> conn = std::make_shared<MongoConnection>(conn_str);
	Logger::Instance()->Debug("Able to make connection: {}", conn->IsConnected());

	std::string test_data_dir = reader->Get(config_section, "test_data_dir", "Failed to find");

	int test_type = reader->GetInteger(config_section, "test_type", -1);
	std::vector<int> d(1, -1);
	std::vector<int> test_items = reader->GetIntegerVector(config_section, "request_items", d);

	test_logger->Info("Configs from ini:"
					  "\n\ttest_data_dir: {}"
					  "\n\ttest_type: {}"
					  "\n\tnumber of test_items: {}", test_data_dir, test_type, test_items.size());


	if (strcmp(test_data_dir.c_str(), "Failed to find") == 0 || test_type == -1 || test_items == d) {
		test_logger->Fatal("Issue parsing test ini, make sure all test fields are valid in section {}", config_section);
		return -1;
	}
	reader->set_default_section(config_section);


	/**
	 * End of test set-up
	 * */


	DIR *dir;
	struct dirent *ent;
	std::string test_data_path = test_data_dir;


	/**
	 * Quantitative analysis tools
	 * */
	double accuracy = 0.0;
	int totalNumDispenses = 0;

	//input path to directory containing saved RGB+Depth data folders here
	if ((dir = opendir(test_data_dir.c_str())) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			if (strcmp(".", ent->d_name) == 0 || strcmp("..", ent->d_name) == 0) continue;
			test_data_path = make_media::paths::join_as_path(test_data_dir, ent->d_name);

			try {
				if (test_type == 0)  // run pre-drop algorithm using saved request.json file
				{
					std::cout << std::endl;
					std::cout << std::endl;
					test_data_path = make_media::paths::join_as_path(test_data_path, "Drop_Target_Image");

					test_logger->Info("Offline Pre-Drop Test Using Saved .json requests");

					std::shared_ptr<std::vector<std::shared_ptr<std::string>>> timestamp_dirs = FileSystemUtil::get_subdirs_in_directory(
							test_data_path);
					for (auto td: *timestamp_dirs) {
						test_logger->Info("Starting Simulation of directory: {}", *td);
						//Todo: add check for color image being present before calling test_image_routine
						test_pre_drop_routine_json(td, reader);
					}
				} else if (test_type == 1) //run pre-drop algorithm using simulated items
				{
					std::cout << std::endl;
					std::cout << std::endl;
					test_data_path = make_media::paths::join_as_path(test_data_path, "Drop_Target_Image");
					test_logger->Info("Offline Pre-Drop Test Using Simulated Requests");

					std::shared_ptr<std::vector<std::shared_ptr<std::string>>> timestamp_dirs = FileSystemUtil::get_subdirs_in_directory(
							test_data_path);
					for (auto td: *timestamp_dirs) {
						test_logger->Info("Starting Simulation of directory: {}", *td);
						//Todo: add check for color image being present before calling test_image_routine
						test_pre_drop_routine_simulated(td, test_items, reader);
					}
				} else if (test_type == 2) //run post-drop algorithm for checking MaxZ height
				{
					std::cout << std::endl;
					std::cout << std::endl;
					std::string exact_test_data_path = make_media::paths::join_as_path(test_data_path,
																					   "Post_Drop_Image");
					std::shared_ptr<std::vector<std::shared_ptr<std::string>>> timestamp_dirs = FileSystemUtil::get_subdirs_in_directory(
							exact_test_data_path);
					for (auto td: *timestamp_dirs) {
						test_logger->Info("Offline Post-Drop Test: Starting Simulation of directory: {}", *td);
						//Todo: add check for color image being present before calling test_image_routine
						test_post_drop_routine(std::make_shared<std::string>(test_data_path), td, reader);
					}
				} else if (test_type == 3) //run pre-drop w/json followed by post-drop for performance evaluation
				{
					/**
					 *   FOR OFFLINE TESTING, FIRST SHOW PREDROP IMAGE //TODO: separate function!
					 */
					std::cout << std::endl;
					std::cout << std::endl;

					test_data_path = make_media::paths::join_as_path(test_data_path, "Drop_Target_Image");
					test_logger->Info("Offline Pre-Drop Test Using Saved .json requests");

					std::shared_ptr<std::vector<std::shared_ptr<std::string>>> timestamp_dirs = FileSystemUtil::get_subdirs_in_directory(
							test_data_path);
					for (auto td: *timestamp_dirs) {
						test_logger->Info("Starting Simulation of directory: {}", *td);
						//Todo: add check for color image being present before calling test_image_routine
						test_pre_drop_routine_json(td, reader);
					}
					// Back track to start before iterating over the pre drop images
					test_data_path = make_media::paths::join_as_path(test_data_dir, ent->d_name);

					std::cout << std::endl;
					std::cout << std::endl;
					std::string exact_test_data_path = make_media::paths::join_as_path(test_data_path,
																					   "Post_Drop_Image");
					test_logger->Info("Offline Post-Drop Test: Starting Simulation of directory: {}",
									  exact_test_data_path);
					//Todo: add check for color image being present before calling test_image_routine
					test_post_drop_routine(std::make_shared<std::string>(test_data_path),
										   std::make_shared<std::string>(exact_test_data_path), reader);
				} else if (test_type == 4) //run pre/post comparison
				{
					/**
					 *   FOR OFFLINE TESTING, FIRST SHOW DropTarget IMAGE //TODO: separate function!
					 */
					std::cout << std::endl;
					std::cout << std::endl;
					test_data_path = make_media::paths::join_as_path(test_data_path, "Pre_Drop_Image");
					std::shared_ptr<std::vector<std::shared_ptr<std::string>>> timestamp_dirs = FileSystemUtil::get_subdirs_in_directory(
							test_data_path);
					test_logger->Info("Pre drop directory is: {}", *(timestamp_dirs->at(
							0))); //Todo: eventually we will want to cycle through each pre-image if there are multiple

					std::string test_data_path_2 = make_media::paths::join_as_path(test_data_dir, ent->d_name,
																				   "Post_Drop_Image");
					std::shared_ptr<std::vector<std::shared_ptr<std::string>>> timestamp_dirs_2 = FileSystemUtil::get_subdirs_in_directory(
							test_data_path_2);
					test_logger->Info("Post drop directory is: {}", *(timestamp_dirs_2->at(0)));

					std::string test_data_path_3 = make_media::paths::join_as_path(test_data_dir, ent->d_name,
																				   "Drop_Target_Image");
					std::shared_ptr<std::vector<std::shared_ptr<std::string>>> timestamp_dirs_3 = FileSystemUtil::get_subdirs_in_directory(
							test_data_path_3);
					test_logger->Info("Drop Target directory is: {}", *(timestamp_dirs_3->at(0)));

					test_logger->Debug("Calling test_compare_pre_post in offline_test.cpp");
					totalNumDispenses++;
					int res = test_compare_pre_post(timestamp_dirs->at(0), timestamp_dirs_2->at(0),
													timestamp_dirs_3->at(0),
													reader);
					accuracy += res;
				}
			}
			catch (const std::exception &e) {
				Logger::Instance()->Error("Error occurred during testing of image data:\n{}", e.what());
			}
			catch (...) {
				test_logger->Error("Unspecified error occurred during testing of image data");
			}
		}
		closedir(dir);
		Logger::Instance()->Debug("accuracy across dir: {}", accuracy / totalNumDispenses);
	} else {
		// could not open directory //
		std::string emsg = "That directory (" + test_data_dir + ") does not appear to exist!";
		perror(emsg.c_str());
		return EXIT_FAILURE;
	}
	return 0;
}
