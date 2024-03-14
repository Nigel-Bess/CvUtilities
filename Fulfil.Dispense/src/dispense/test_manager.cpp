#include <memory>
#include <exception>
#include <stdexcept>
#include <Fulfil.Dispense/dispense/test_manager.h>

#include <Fulfil.Dispense/dispense/realsense_manager.h>
#include <Fulfil.Dispense/dispense/realsense_runner_factory.h>
#include <Fulfil.Dispense/dispense/realsense_bay_parser.h>
#include <Fulfil.Dispense/bays/bay_manager.h>

#include <Fulfil.DepthCam/core.h>

#include <Fulfil.CPPUtils/inih/INIReader.h>
#include <Fulfil.CPPUtils/logging.h>

#include <Fulfil.CPPUtils/networking/socket_network_manager.h>

#include <FulfilMongoCpp/mongo_connection.h>
#include <FulfilMongoCpp/mongo_objects/mongo_document.h>

#include "Fulfil.Dispense/mongo/mongo_tray_calibration.h"

#include "Fulfil.Dispense/dispense/dispense_manager.h"
#include "Fulfil.Dispense/visualization/make_media.h"
#include <Fulfil.CPPUtils/file_system_util.h>
#include <Fulfil.CPPUtils/timer.h>
#include <Fulfil.DepthCam/data.h>
#include <Fulfil.DepthCam/data/bigquery_upload.h>
#include <Fulfil.DepthCam/mocks.h>
#include <Fulfil.Dispense/commands/error_response.h>
#include <Fulfil.Dispense/commands/parsing/dispense_json_parser.h>
#include <Fulfil.Dispense/commands/parsing/dispense_request_parser.h>
//#include <Fulfil.Dispense/dispense/dispense_processing_queue_predicate.h>
//#include <Fulfil.Dispense/dispense/image_persistence/realsense_file_manager.h>
//#include <Fulfil.Dispense/dispense/image_persistence/realsense_image_persistence_manager.h>
//#include <Fulfil.Dispense/dispense/image_persistence/realsense_timestamper.h>
#include <Fulfil.Dispense/tray/item_edge_distance_result.h>
#include <Fulfil.Dispense/tray/tray_algorithm.h>
#include <Fulfil.Dispense/tray/tray_result.h>
#include <Fulfil.Dispense/visualization/live_viewer.h>
#include <FulfilMongoCpp/mongo_filter/mongo_filters.h>
#include <FulfilMongoCpp/mongo_json/mongo_json_document.h>
#include <FulfilMongoCpp/mongo_objects/mongo_basic_document.h>
#include <FulfilMongoCpp/mongo_parse/mongo_bsonxx_encoder.h>
#include <Fulfil.Dispense/commands/code_response.h>
#include <Fulfil.Dispense/commands/content_response.h>
#include <Fulfil.Dispense/commands/dispense_command.h>
#include <Fulfil.Dispense/mongo/lfb_config.h>

using fulfil::depthcam::data::BQUpload;
using ff_mongo_cpp::MongoConnection;
using ff_mongo_cpp::mongo_parse::MongoBsonxxEncoder;
using ff_mongo_cpp::mongo_objects::MongoJsonDocument;
using ff_mongo_cpp::mongo_objects::MongoBasicDocument;
using fulfil::mongo::MongoTrayCalibration;
using fulfil::utils::FileSystemUtil;
using fulfil::dispense::DispenseManager;
using fulfil::dispense::commands::DispenseResponse;
using fulfil::dispense::commands::DispenseRequest;
using fulfil::depthcam::Session;
using fulfil::depthcam::pointcloud::PointCloud;
using fulfil::dispense::drop::DropResult;
using fulfil::dispense::commands::DropTargetDetails;
using fulfil::utils::networking::SocketInformation;
using fulfil::utils::networking::SocketManager;
using fulfil::depthcam::aruco::MarkerDetector;
using fulfil::depthcam::aruco::Container;
using fulfil::depthcam::aruco::MarkerDetectorContainer;
using fulfil::dispense::drop::DropManager;
using ff_mongo_cpp::mongo_objects::MongoObjectID;
using fulfil::dispense::commands::DispenseRequestParser;
using fulfil::dispense::commands::DispenseJsonParser;
using fulfil::utils::networking::SocketNetworkManager;
using fulfil::utils::processingqueue::ProcessingQueue;
using fulfil::dispense::commands::ErrorResponse;
using fulfil::utils::processingqueue::ProcessingQueuePredicate;
using fulfil::depthcam::data::DataGenerator;
using fulfil::depthcam::data::GCSSender;
using fulfil::depthcam::data::FileSender;
using fulfil::depthcam::data::UploadGenerator;
using fulfil::dispense::tray_processing::TrayAlgorithm;
using fulfil::dispense::tray::ItemEdgeDistanceResult;
using fulfil::dispense::tray::TrayResult;
using fulfil::dispense::tray::Tray;
using fulfil::utils::Logger;
using fulfil::utils::ini::IniSectionReader;
using fulfil::dispense::commands::TrayValidationResponse;
using fulfil::dispense::commands::ItemEdgeDistanceResponse;
using fulfil::dispense::commands::PostLFRResponse;
using fulfil::dispense::visualization::LiveViewer;
using fulfil::dispense::visualization::ViewerImageType;
using fulfil::depthcam::mocks::MockSession;
using ff_mongo_cpp::mongo_filter::MongoFilter;
using ff_mongo_cpp::MongoConnection;
using ff_mongo_cpp::mongo_objects::MongoDocument;
using fulfil::utils::Logger;
using fulfil::dispense::RealsenseManager;
using fulfil::depthcam::DeviceManager;
using fulfil::dispense::RealsenseRunnerFactory;
using fulfil::dispense::RealsenseBayParser;
using fulfil::dispense::bays::BayManager;
using fulfil::depthcam::Session;
using fulfil::mongo::MongoTrayCalibration;
using fulfil::utils::networking::SocketInformation;
using fulfil::utils::networking::SocketManager;
using fulfil::dispense::commands::DispenseRequestParser;
using fulfil::dispense::commands::DispenseJsonParser;
using fulfil::utils::processingqueue::ProcessingQueue;
using fulfil::dispense::TestManager;
using fulfil::depthcam::DepthSensor;

using fulfil::dispense::commands::DispenseCommand;

using namespace rs2;
    
TestManager::TestManager(){
    std::shared_ptr<SocketInformation> socket_info = std::make_shared<SocketInformation>(9510);
    std::shared_ptr<SocketManager> socket_manager = std::make_shared<SocketManager>(socket_info);  
    std::shared_ptr<DispenseRequestParser> parser = std::make_shared<DispenseRequestParser>(std::make_shared<DispenseJsonParser>());
    network_manager = std::make_shared<SocketNetworkManager<std::shared_ptr<DispenseRequest>>>(socket_manager, parser);
    processing_queue = std::make_shared<ProcessingQueue<std::shared_ptr<DispenseRequest>, std::shared_ptr<DispenseResponse>>>();
    this->mongo_bag_state = std::make_shared<fulfil::mongo::MongoBagState>();
    //bind_delegates();
    //network_manager->start_listening();
    context ctx;
    std::vector<DepthSensor> pipes;
    auto list = ctx.query_devices();
    std::cout << "Found " << list.size() << " RealSense devices" << std::endl;
    for(auto r = 0; r < list.size(); r++){
        
        std::thread run(&TestManager::run_stream, this, list[r].get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));
        run.detach();

    }

}

void TestManager::run_stream(std::string serial){
    static int id = 1;
    DepthSensor sensor(serial);
    sensor.name_ = std::to_string(id++);

    std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    printf("run_stream starting\n");
    auto count = 1;
    while(true){
        auto start = std::chrono::system_clock::now();
        auto frame = sensor.get_latest_frame();
        auto color = frame.get_color_frame();
        auto mat = cv::Mat(cv::Size(color.get_width(), color.get_height()),
                            CV_8UC3, (void*)color.get_data(), cv::Mat::AUTO_STEP);
        std::string fname("/media/fulfil/6335-3031/" + serial + std::to_string(count++) + ".jpg");
        auto success = cv::imwrite(fname, mat);
        auto elapsed = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count();
        std::cout << "Took " << sensor.name_ << " [" << serial << "] " << std::to_string(elapsed) << "ms to get image";
        std::cout << " " << fname << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        //return;
    }
    // rs2::pipeline p;
    // rs2::config cfg;
    // cfg.enable_device(serial);
    // cfg.enable_stream(RS2_STREAM_COLOR, 1280, 720, RS2_FORMAT_BGR8, 15);
    // cfg.enable_stream(RS2_STREAM_DEPTH, 1280, 720, RS2_FORMAT_Z16, 15);
    // p.start(cfg);
    // while(true){
    //     try{
    //         rs2::frameset f = p.wait_for_frames();
    //         auto elapsed = duration_cast<milliseconds>(std::chrono::system_clock::now() - clock).count();
    //         printf("%s [%8ld] %08.02f %f\n",serial.c_str(), elapsed, f.get_timestamp(), f.get_depth_frame().get_distance(0,0));
    //         clock = std::chrono::system_clock::now();
    //     }
    //     catch (const rs2::unrecoverable_error& e){
    //         Logger::Instance()->Fatal("Unrecoverable:\nRealsense Exception {}\nIn function {}\nWith args {}",
    //                                    e.what(), e.get_failed_function(), e.get_failed_args());
    //         //unrecoverable_exc++;
    //     }
    //     catch (const rs2::recoverable_error& e){
    //         Logger::Instance()->Error("Recoverable:\nRealsense Exception {}\nIn function {}\nWith args {}",
    //                                    e.what(), e.get_failed_function(), e.get_failed_args());
    //         //recoverable_exc++;
    //     }
    //     catch (const std::exception & e){
    //         Logger::Instance()->Error("DepthSensor::manage_pipe exception {}", e.what());
    //         //std_exceptions++;
    //     }
    //     catch(...){
    //         Logger::Instance()->Error("DepthSensor::manage_pipe unknown error!");
    //         //std_exceptions++;
    //     }

    // }
}
void TestManager::start(){
    network_manager->start_listening();
    processing_queue->start_processing(5);
}

void TestManager::bind_delegates(){
    network_manager->delegate =  this->weak_from_this();
    processing_queue->delegate =  this->weak_from_this();
}

void TestManager::did_receive_request(std::shared_ptr<DispenseRequest> request){
    request->delegate = this->weak_from_this();
    Logger::Instance()->Info("Test Manager: about to push request to process queue");
    this->processing_queue->push(request);
}

void TestManager::send_response(std::shared_ptr<DispenseResponse> response){   
    
    Logger::Instance()->Info("send response");
    this->network_manager->send_response(response);
}

std::shared_ptr<DispenseResponse> TestManager::process_request(std::shared_ptr<DispenseRequest> request){
    Logger::Instance()->Info("execute request");
    return request->execute();
}

void TestManager::did_receive_invalid_request(std::shared_ptr<std::string> request_id){
    this->network_manager->send_response(std::make_shared<ErrorResponse>(request_id));
}

void TestManager::did_receive_invalid_request(){
    this->network_manager->send_response(std::make_shared<ErrorResponse>());
}


int TestManager::handle_update_state(std::shared_ptr<std::string> PrimaryKeyID, std::shared_ptr<nlohmann::json> request_json)
{
  Logger::Instance()->Info("Handle Update State called in Test Manager");
  //this->bot_already_rotated_for_current_dispense = false; //get state indicates a new bot, reset this flag here
  try {

    auto bag = nlohmann::to_string((*request_json)["Bag_State"]);
    std::cout << "CvBagState JSON from FC: " << bag << std::endl;
    auto cvbag = fulfil::mongo::CvBagState((*request_json)["Bag_State"]);
    this->mongo_bag_state->parse_in_values((std::make_shared<fulfil::mongo::CvBagState>(cvbag)));
    return 0;
  }
  catch(const std::exception & e) //TODO: update error handling here with different parsing / building errors
  {
      Logger::Instance()->Error("Issue handling get state request: \n\t{}", e.what());
      return 1;
  }
  catch(...)
  {
      Logger::Instance()->Error("Unspecified error in handle_get_state");
      return 1;
  }
}

std::string TestManager::handle_get_state(std::shared_ptr<std::string> PrimaryKeyID, std::shared_ptr<nlohmann::json> request_json)
{
    Logger::Instance()->Trace("Handle Update State called in Dispense Manager");
    try
    {
        if(this->mongo_bag_state->raw_mongo_doc == nullptr)this->mongo_bag_state->raw_mongo_doc = std::make_shared<fulfil::mongo::CvBagState>();
        
        return this->mongo_bag_state->raw_mongo_doc->ToString();

    }
    catch(const std::exception & e)
    {
        Logger::Instance()->Error("Issue handling update state request: \n\t{}", e.what());
        return "-1";
    }
    catch(...)
    {
        return "-2";
    }
}

void TestManager::handle_request(std::shared_ptr<std::string> payload, std::shared_ptr<std::string> command_id){
    std::cout << "payload---> " << payload.get()->c_str() << std::endl;
    auto request_json = std::make_shared<nlohmann::json>(nlohmann::json::parse(payload->c_str()));
    auto type = (*request_json)["Type"].get<fulfil::dispense::commands::DispenseCommand>();
    auto pkid =  std::make_shared<std::string>((*request_json)["Primary_Key_ID"].get<std::string>());
    std::shared_ptr<DispenseResponse> response;
    switch(type){
        case DispenseCommand::request_bag_state:{
            Logger::Instance()->Info("Received Get State Request, PKID: {}, request_id: {}", *pkid, *command_id);
            auto result = handle_get_state(pkid, request_json);
            response = std::make_shared<fulfil::dispense::commands::ContentResponse>(
                        command_id, std::make_shared<std::string>(result), DepthCameras::MessageType::MESSAGE_TYPE_BAG_STATE_REQUEST);
            break;
        }
        case DispenseCommand::send_bag_state:{
            Logger::Instance()->Info("Received Update State Request, PKID: {}, request_id: {}",  *pkid, *command_id);
            auto result = handle_update_state(pkid, request_json);
            response = std::make_shared<fulfil::dispense::commands::CodeResponse>(command_id, result);
            break;
        }
        default:
            Logger::Instance()->Error("Un-handled request type, PKID: {}, request_id: {}", *pkid, *command_id);
            response = std::make_shared<fulfil::dispense::commands::CodeResponse>(command_id, (int)type);
            break;
    }
    network_manager->send_response(response);    
}