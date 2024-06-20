#include <memory>

#include <Fulfil.CPPUtils/file_system_util.h>
#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.CPPUtils/networking/socket_network_manager.h>
#include <Fulfil.CPPUtils/timer.h>
#include <Fulfil.DepthCam/data.h>
#include <Fulfil.DepthCam/data/bigquery_upload.h>
#include <Fulfil.DepthCam/mocks.h>
#include <Fulfil.Dispense/commands/code_response.h>
#include <Fulfil.Dispense/commands/content_response.h>
#include <Fulfil.Dispense/commands/dispense_command.h>
#include <Fulfil.Dispense/commands/drop_target/drop_target_response.h>
#include <Fulfil.Dispense/commands/error_response.h>
#include <Fulfil.Dispense/commands/parsing/dispense_json_parser.h>
#include <Fulfil.Dispense/commands/parsing/dispense_request_parser.h>
#include "Fulfil.Dispense/dispense/dispense_manager.h"
#include <Fulfil.Dispense/dispense/dispense_processing_queue_predicate.h>
#include "Fulfil.Dispense/dispense/drop_error_codes.h"
#include <Fulfil.Dispense/mongo/mongo_tray_calibration.h>
#include <Fulfil.Dispense/tray/item_edge_distance_result.h>
#include <Fulfil.Dispense/tray/tray_algorithm.h>
#include "Fulfil.Dispense/commands/parsing/tray_parser.h"
#include <Fulfil.Dispense/tray/tray_result.h>
#include <Fulfil.Dispense/visualization/live_viewer.h>
#include "Fulfil.Dispense/visualization/make_media.h"

using ff_mongo_cpp::MongoConnection;
using ff_mongo_cpp::mongo_objects::MongoObjectID;
using fulfil::depthcam::Session;
using fulfil::depthcam::pointcloud::PointCloud;
using fulfil::dispense::commands::DropTargetDetails;
using fulfil::depthcam::aruco::MarkerDetector;
using fulfil::depthcam::aruco::Container;
using fulfil::depthcam::aruco::MarkerDetectorContainer;
using fulfil::depthcam::data::BQUpload;
using fulfil::depthcam::data::DataGenerator;
using fulfil::depthcam::data::GCSSender;
using fulfil::depthcam::data::FileSender;
using fulfil::depthcam::data::UploadGenerator;
using fulfil::depthcam::mocks::MockSession;
using fulfil::dispense::commands::DispenseCommand;
using fulfil::dispense::commands::DispenseJsonParser;
using fulfil::dispense::commands::DispenseRequest;
using fulfil::dispense::commands::DispenseRequestParser;
using fulfil::dispense::commands::DispenseResponse;
using fulfil::dispense::commands::ErrorResponse;
using fulfil::dispense::commands::ItemEdgeDistanceResponse;
using fulfil::dispense::commands::PostLFRResponse;
using fulfil::dispense::commands::TrayValidationResponse;
using fulfil::dispense::DispenseManager;
using fulfil::dispense::drop::DropManager;
using fulfil::dispense::drop::DropResult;
using fulfil::dispense::drop_target_error_codes::DropTargetErrorCodes;
using fulfil::dispense::drop_target_error_codes::get_error_name_from_code;
using fulfil::dispense::tray_processing::TrayAlgorithm;
using fulfil::dispense::tray::ItemEdgeDistanceResult;
using fulfil::dispense::tray::Tray;
using fulfil::dispense::tray::TrayResult;
using fulfil::dispense::visualization::LiveViewer;
using fulfil::dispense::visualization::ViewerImageType;
using fulfil::mongo::MongoTrayCalibration;
using fulfil::utils::FileSystemUtil;
using fulfil::utils::ini::IniSectionReader;
using fulfil::utils::Logger;
using fulfil::utils::networking::SocketInformation;
using fulfil::utils::networking::SocketManager;
using fulfil::utils::networking::SocketNetworkManager;
using fulfil::utils::processingqueue::ProcessingQueue;
using fulfil::utils::processingqueue::ProcessingQueuePredicate;

DispenseManager::DispenseManager(
  int bay, std::shared_ptr<fulfil::depthcam::Session> LFB_session,
  std::shared_ptr<fulfil::depthcam::Session> tray_session,
  std::shared_ptr<INIReader> dispense_man_reader,
  std::shared_ptr<INIReader> tray_config_reader,
  std::shared_ptr<MongoConnection> mongo_conn,
  std::shared_ptr<fulfil::dispense::tray::TrayManager> tray_manager) : bay(bay),
        dispense_reader(dispense_man_reader), tray_config_reader(tray_config_reader), mongo_connection(mongo_conn)
  {
  Logger::Instance()->Trace("DispenseManager Constructor Called");

  // TODO was this a weird merge conflict or can we get rid of these
//      this->dispense_man_reader = dispense_man_reader;
//      this->tray_config_reader =  tray_config_reader;

  //setting up networking stuff
  // TODO Once needs stabilize, we should probably just add data members in reader to interface
  // This should be the final place that the reader pointer is passed!
  auto dispense_name = std::string("dispense_") + char(this->bay + 48);
  auto safe_get_dispense_string_val = [this, dispense_name=std::string("dispense_") + char(this->bay + 48)]
          (auto key, std::string default_value) {
            return this->dispense_reader->Get(dispense_name, key, default_value);
  };
  this->machine_name = safe_get_dispense_string_val( "name", dispense_name);
  this->machine_id = safe_get_dispense_string_val("_id", "000000000000000000000000");
  int port = this->dispense_reader->GetInteger(dispense_name, "port", 9500);

  this->motion_config_reader = INIReader("motion_config.ini", true);


  auto safe_get_tray_string_val =
          [&tray_dim_type = this->tray_dimension_type, &tray_config=this->tray_config_reader](auto key, std::string default_value) {
              return tray_config->Get(tray_dim_type, key, default_value); };
  this->store_id = safe_get_tray_string_val("store_id", "f0");
  this->cloud_media_bucket = safe_get_tray_string_val("cloud_media_bucket", "factory-media");

  Logger::Instance()->Info("Initializing socket network manager at configured port {} for machine {}", port, this->machine_name);

  /**
   *  Initialize connection to LFB camera-on-rail motor if motion control is required
   */

  if(this->dispense_reader->GetBoolean(dispense_man_reader->get_default_section(), "motion_control_required", false))
  {
    this->motor_in_position = false;
    Logger::Instance()->Info("Motion Control IS Required to run this bay!");
    uint32_t baud = this->motion_config_reader.GetInteger("motion_parameters", "baud", -1);
    std::string com_port = this->dispense_reader->Get(dispense_man_reader->get_default_section(), "trinamic_com_port", "error");
    this->motion_controller = std::make_shared<fulfil::dispense::motion::Trinamic>(baud, com_port.c_str());
    this->motion_controller->TryConnectMotor();
    this->motion_controller->StopMotor();

    bool successfully_set_params = set_motion_params();
    if(!successfully_set_params)
    {
      Logger::Instance()->Error("Motion control parameters did not load properly");
      exit(14); // TODO error map for the dispense manager generally
    }
    motor_in_position = true;
  }
  else
  {
    motor_in_position = true;
    Logger::Instance()->Info("Motion Control is NOT Required to run this bay!");
  }

  if(tray_session)
  {
    this->tray_session = tray_session;
    //find last tray camera calibration from Mongo collection Machines.DepthCamera
    if (dispense_man_reader->GetBoolean(dispense_man_reader->get_default_section(), "validate_calibration_with_mongo", false))
    {
      MongoTrayCalibration temp;
      this->tray_calibration_ids = temp.findLastTrayCalibration(
              this->machine_id, this->mongo_connection, this->tray_config_reader);
    }

    //read specific tray configuration
    std::string tray_config_type = this->dispense_reader->Get("device_specific", "tray_config_type");
    this->tray_dimension_type = "tray_dimensions_" + tray_config_type;
    Logger::Instance()->Info("AGX specific ini file indicates to use tray config section {}", this->tray_dimension_type);

    this->tray_manager = tray_manager;
  }

  if(LFB_session)
  {
      //Sensor is connected
      Logger::Instance()->Trace("LFB session.");
      this->LFB_session = LFB_session;

      if(dispense_man_reader->GetBoolean(dispense_man_reader->get_default_section(), "live_visualize_drop", false))
      {
          std::string live_viewer_path = this->dispense_reader->Get(dispense_man_reader->get_default_section(), "live_visualize_drop_path", "");
          if(live_viewer_path.empty()) {
            Logger::Instance()->Warn("Live Viewer Drop Output Path not specified. Check main.ini file");
          } else {
              std::string cloud_directory = make_media::paths::join_as_path(this->store_id, "dispenses").string();
              std::shared_ptr<GCSSender> gcs_sender =  std::make_shared<GCSSender>(this->cloud_media_bucket, cloud_directory);
              this->live_viewer = std::make_shared<LiveViewer>(live_viewer_path, gcs_sender);
              Logger::Instance()->Debug("Live Viewer on Drop for bay: {}, set up with base path: {}", bay, live_viewer_path);
          }
      }
  }

  this->drop_manager = std::make_shared<DropManager>(this->LFB_session, dispense_man_reader, this->live_viewer);
  
  std::shared_ptr<DispenseRequestParser> parser = std::make_shared<DispenseRequestParser>(std::make_shared<DispenseJsonParser>());
  std::shared_ptr<SocketManager> socket_manager = std::make_shared<SocketManager>(std::make_shared<SocketInformation>(port));

  this->network_manager = std::make_shared<SocketNetworkManager<std::shared_ptr<DispenseRequest>>>(socket_manager, parser);

  this->processing_queue = std::make_shared<ProcessingQueue<std::shared_ptr<DispenseRequest>, std::shared_ptr<DispenseResponse>>>();

  // separate uploader for keeping track of if we should stop saving the LFB video
  //this->stop_lfb_video_saving = std::make_shared<bool>(true);
  //this->stop_lfb_video_delay = std::make_shared<int>(0);
  this->lfb_upload_generator = std::make_shared<UploadGenerator>(0);

  // separate uploader for keeping track of if we should stop saving the tray video
  //this->stop_tray_video_saving = std::make_shared<bool>(true);
  //this->stop_tray_video_delay = std::make_shared<int>(0);
  this->tray_upload_generator = std::make_shared<UploadGenerator>(0);

  if(this->LFB_session != nullptr) {
      this->LFB_session->set_emitter(false); //by default turn the LFB projector emitter to the OFF state
  }

  if(this->tray_session != nullptr) {
      this->tray_session->set_emitter(false); //by default turn the tray projector emitter to the OFF state
  }
}

void DispenseManager::bind_delegates()
{
    this->network_manager->delegate = this->weak_from_this();
    this->processing_queue->delegate = this->weak_from_this();
}

void DispenseManager::start()
{
    Logger::Instance()->Info("Starting bay {}: {}",this->bay, this->machine_name);
    this->network_manager->start_listening();
    // this->processing_queue->start_processing(5);
    while(true)std::this_thread::sleep_for(std::chrono::seconds(10));
}

void DispenseManager::handle_request(std::shared_ptr<std::string> payload, std::shared_ptr<std::string> command_id){
    std::thread(&DispenseManager::handle_request_in_thread, this, payload, command_id).detach();
}

void DispenseManager::handle_request_in_thread(std::shared_ptr<std::string> payload, std::shared_ptr<std::string> command_id){
    std::cout << "payload---> " << payload.get()->c_str() << std::endl;
    auto request_json = std::make_shared<nlohmann::json>(nlohmann::json::parse(payload->c_str()));
    auto type = (*request_json)["Type"].get<fulfil::dispense::commands::DispenseCommand>();
    auto pkid =  std::make_shared<std::string>((*request_json)["Primary_Key_ID"].get<std::string>());
    std::shared_ptr<DispenseResponse> response;
    switch(type){
        case DispenseCommand::request_bag_state:{
            Logger::Instance()->Info("Received Get State Request on Bay {}, PKID: {}, request_id: {}", this->machine_name, *pkid, *command_id);
            auto result = handle_get_state(pkid, request_json);
            response = std::make_shared<fulfil::dispense::commands::ContentResponse>(
                        command_id, std::make_shared<std::string>(result), DepthCameras::MessageType::MESSAGE_TYPE_BAG_STATE_REQUEST);
            break;
        }
        case DispenseCommand::send_bag_state:{
            Logger::Instance()->Info("Received Update State Request on Bay {}, PKID: {}, request_id: {}", this->machine_name,  *pkid, *command_id);
            auto result = handle_update_state(pkid, request_json);
            response = std::make_shared<fulfil::dispense::commands::CodeResponse>(command_id, result);
            break;
        }
        default:
            Logger::Instance()->Error("Un-handled request type on Bay {}, PKID: {}, request_id: {}", this->machine_name, *pkid, *command_id);
            response = std::make_shared<fulfil::dispense::commands::CodeResponse>(command_id, (int)type);
            break;
        case DispenseCommand::drop_target:{
            Logger::Instance()->Info("Received Drop Target Request on Bay {}, PKID: {}, request_id: {}", this->machine_name, *pkid, *command_id);
            auto drop_details = std::make_shared<fulfil::dispense::commands::DropTargetDetails>(request_json, command_id);
            auto raw_result = handle_drop_target(drop_details, request_json);
             if(raw_result->success_code == 0 || raw_result->success_code == 9){
                response = std::make_shared<fulfil::dispense::commands::DropTargetResponse>(command_id, raw_result->success_code,
                            raw_result->rover_position, raw_result->dispense_position, raw_result->depth_result,
                            raw_result->max_depth_point_X, raw_result->max_depth_point_Y, raw_result->max_Z,
                            raw_result->Rotate_LFB, raw_result->LFB_Currently_Rotated, raw_result->Swing_Collision_Expected,
                            raw_result->target_depth_range, raw_result->target_depth_variance, raw_result->interference_max_z,
                            raw_result->interference_average_z, raw_result->target_region_max_z, raw_result->error_description);
             }
             else response = std::make_shared<fulfil::dispense::commands::DropTargetResponse>(command_id, raw_result->success_code, raw_result->error_description);
            break;
        }
        case DispenseCommand::pre_LFR:{
            Logger::Instance()->Info("Received Pre Drop LFB Request on Bay {}, PKID: {}, request_id: {}", this->machine_name, *pkid, *command_id);
            auto code = handle_pre_LFR(pkid, request_json);
            response = std::make_shared<fulfil::dispense::commands::CodeResponse>(command_id, code);
            break;
        }
        case DispenseCommand::post_LFR:{
            Logger::Instance()->Info("Received Post Drop Request on Bay {}, PKID: {}, request_id: {}", this->machine_name, *pkid, *command_id);
            response =  handle_post_LFR(pkid, command_id, request_json);
            break;
        }
        case DispenseCommand::side_dispense_target:{
            Logger::Instance()->Info("Received Side Dispense Target Request on Bay {}, PKID: {}, request_id: {}", this->machine_name, *pkid, *command_id);
            std::shared_ptr<std::string> base_directory = this->create_datagenerator_basedir();

            std::shared_ptr<std::string> time_stamp_string = FileSystemUtil::create_datetime_string();
            DataGenerator generator = DataGenerator(this->lfb_session,
                                                    this->create_datagenerator_basedir()),
                                                    request_json);
            generator.save_data(time_stamp);
            response = std::make_shared<fulfil::dispense::commands::SideDispenseTargetResponse>(command_id);
            break;
        }
        case DispenseCommand::pre_side_dispense:{
            Logger::Instance()->Info("Received Pre Side Dispense Request on Bay {}, PKID: {}, request_id: {}", this->machine_name, *pkid, *command_id);
            response = std::make_shared<fulfil::dispense::commands::CodeResponse>(command_id, code);
            break;
        }
        case DispenseCommand::post_side_dispense:{
            Logger::Instance()->Info("Received Post Side Dispense Request on Bay {}, PKID: {}, request_id: {}", this->machine_name, *pkid, *command_id);
            response = std::make_shared<fulfil::dispense::commands::PostSideDispenseResponse>(command_id);
            break;
        }
        case DispenseCommand::start_lfb_video:{
                Logger::Instance()->Info("Received Start LFB Video Request on Bay {}, PKID: {}, request_id: {}", this->machine_name, *command_id);
            handle_start_lfb_video(pkid);
            response = std::make_shared<fulfil::dispense::commands::CodeResponse>(command_id, 0);
            break;
        }
        case DispenseCommand::stop_lfb_video:{
            Logger::Instance()->Info("Received Stop LFB Video Request on Bay {}, PKID: {}, request_id: {}", this->machine_name, *pkid, *command_id);
            handle_stop_lfb_video(); 
            response = std::make_shared<fulfil::dispense::commands::CodeResponse>(command_id, 0);
            break;
        }
        case DispenseCommand::start_tray_video:{
            Logger::Instance()->Info("Received Start Tray Video Request on Bay {}, PKID: {}, request_id: {}", this->machine_name, *pkid, *command_id);
            handle_start_tray_video(pkid); 
            response = std::make_shared<fulfil::dispense::commands::CodeResponse>(command_id, 0);
            break;
        }
        case DispenseCommand::stop_tray_video:{
            Logger::Instance()->Info("Received Stop Tray Video Request on Bay {}, PKID: {}, request_id: {}", this->machine_name, *pkid, *command_id);
            int delay = 0;
            if (request_json->contains("Delay_Ms")){
                delay = std::min(5000, (*request_json)["Delay_Ms"].get<int>());
                //usleep(1000*delay);
                Logger::Instance()->Debug("Received stop video delay of {} ms", delay);
            }
            handle_stop_tray_video(delay);
            response = std::make_shared<fulfil::dispense::commands::CodeResponse>(command_id, 0);
            break;
        }
        case DispenseCommand::tray_validation:{
            Logger::Instance()->Info("Received Tray Validation Request on Bay {}, PKID: {}, request_id: {}", this->machine_name,  *pkid, *command_id);
            response = handle_tray_validation(command_id, request_json);
            break;
        }
        case DispenseCommand::item_edge_distance:{
            Logger::Instance()->Info("Received Tray Dispense Lane Request on Bay {}, PKID: {}, request_id: {}", this->machine_name, *pkid, *command_id);
            response = handle_item_edge_distance(command_id, request_json);
            break;
        }
    }
    network_manager->send_response(response);    
}

void DispenseManager::did_receive_request(std::shared_ptr<DispenseRequest> request)
{

     Logger::Instance()->Debug("Dispense Manager: about to push request to process queue {}", *request->command_id);
     this->processing_queue->push(request);
}

void DispenseManager::did_receive_invalid_request(std::shared_ptr<std::string> request_id)
{
    this->network_manager->send_response(std::make_shared<ErrorResponse>(request_id));
}

void DispenseManager::did_receive_invalid_request()
{
    this->network_manager->send_response(std::make_shared<ErrorResponse>());
}

void DispenseManager::send_response(std::shared_ptr<DispenseResponse> response)
{
    this->network_manager->send_response(response);
}

std::shared_ptr<DispenseResponse> DispenseManager::process_request(std::shared_ptr<DispenseRequest> request)
{
    return request->execute();
}

bool DispenseManager::set_motion_params()
{
  if (!this->LFB_session) {
    Logger::Instance()->Warn("No LFB Session: Bouncing Motor Control");
    return false;

  }
  //Load in initial motor control parameters
  // ALERT! FOR MOTOR SAFETY DO NOT CHANGE THESE WITHOUT READING TRINAMIC-1241 FIRMWARE MANUAL. DAMAGE TO MOTOR MAY OCCUR
  int microstep_setting = 3; //microstep resolution, value = 2^X. E.g. 2^3 = 8 microsteps per step (full step)
  int steps_per_revolution = 200; //Motor full-step resolution (default is 200). Full steps per revolution

  int microsteps_per_step = pow(2.0, microstep_setting); //microsteps are equivalent to "pulses" in trinamic documentation, e.g. for pps units
  int microsteps_per_rev = microsteps_per_step * steps_per_revolution; //equivalent to pulses per revolution
  Logger::Instance()->Info("Motion control pulses per motor revolution is: {} pulses", microsteps_per_rev);

  // ALERT! FOR MOTOR SAFETY DO NOT CHANGE THESE WITHOUT READING TRINAMIC-1241 FIRMWARE MANUAL. DAMAGE TO MOTOR MAY OCCUR
  bool check1 = motion_controller->SendMotorCommand(0x02, TMCL_SAP, MOT_MaxCurrent, 0, 150); //max current when motor is running
  bool check2 = motion_controller->SendMotorCommand(0x02, TMCL_SAP, MOT_StandbyCurrent, 0, 2); //max current when motor is holding still. Aim for as low as possible so motor can cool down
  bool check3 = motion_controller->SendMotorCommand(0x02, TMCL_SAP, 140, 0, microstep_setting); //microstep resolution, microsteps per full step
  bool check4 = motion_controller->SendMotorCommand(0x02, TMCL_SAP, 202, 0, steps_per_revolution); //motor fullstep resolution

  // These checks are critical to ensure that motor safe parameters have been loaded properly at initialization. Program should not run if fails
  if(!check1 or !check2 or !check3 or !check4)
  {
    Logger::Instance()->Error("Unsuccessful in setting fundamental parameters on Trinamic board during initialization");
    return false;
  }

  // Check for valid trinamic home sensor reading
  int analog_input = motion_controller->GetAnalogSensorInput();
  bool check5 = (analog_input > 1000) and (analog_input < 4100); //expected value when ON is: 4095, off is: ???
  if(!check5)
  {
    Logger::Instance()->Error("Trinamic home sensor input is not within expected range (1000 - 4100), with value: {}. Check wiring!", analog_input);
    return false;
  }
  else
  {
    Logger::Instance()->Info("Trinamic home sensor input is within expected range, value: {}", analog_input);
  }

  // TODO: add check that trinamic enable digital IO is ON
  bool frozen_dispense = this->dispense_reader->GetBoolean("device_specific", "is_frozen", false);
  if(frozen_dispense) Logger::Instance()->Info("Camera-on-rail position speeds will be set based on Frozen VLS motion config");
  int desired_rev_per_second_homing = this->motion_config_reader.GetInteger("motion_parameters", "rev_per_sec_homing", 0);
  int desired_rev_per_second_positioning = frozen_dispense ?
      this->motion_config_reader.GetInteger("motion_parameters", "rev_per_sec_positioning_frozen", 0) :
      this->motion_config_reader.GetInteger("motion_parameters", "rev_per_sec_positioning", 0);

  this->home_velocity_setting = round(microsteps_per_rev * desired_rev_per_second_homing); // in pps
  this->position_velocity_setting = round(microsteps_per_rev * desired_rev_per_second_positioning); // in pps
  Logger::Instance()->Info("Motion control velocity setting for motor homing is: {} pps", home_velocity_setting);
  Logger::Instance()->Info("Motion control velocity setting for motor positioning is: {} pps", position_velocity_setting);

  if(home_velocity_setting <= 0 or home_velocity_setting > 5000 or position_velocity_setting <= 0 or position_velocity_setting > 10000)
  {
    Logger::Instance()->Debug("Motion control velocity settings appear out of expected range");
    return false;
  }

  if(!motion_controller->SendMotorCommand(0x02, TMCL_SAP, MOT_MaxPositionSpeed, 0, position_velocity_setting))
  {
    Logger::Instance()->Error("Unsuccessful in setting parameter on Trinamic board during initialization");
    return false;
  }
  return true;
}

std::shared_ptr<fulfil::dispense::drop::DropResult> DispenseManager::handle_drop_target(std::shared_ptr<fulfil::dispense::commands::DropTargetDetails> details,
                                                                                        std::shared_ptr<nlohmann::json> request_json)
{
  std::string PrimaryKeyID = (*request_json)["Primary_Key_ID"].get<std::string>();
  auto timer = fulfil::utils::timing::Timer("DispenseManager::handle_drop_target for " + this->machine_name + " request " + PrimaryKeyID);
  Logger::Instance()->Debug("Handling Drop Target Command {} for Bay: {}", PrimaryKeyID, this->machine_name);
  if (!this->LFB_session) {
    Logger::Instance()->Warn("No LFB Session: Bouncing Drop Camera Drop Target");
    return std::make_shared<DropResult>(details->request_id, DropTargetErrorCodes::AlgorithmFail_Bypass,
                                        "No LFB Session: Bouncing Drop Camera Drop Target"); // Todo: move to throw/catch format, log data
  }
  //set cached drop_target fields back to nullptr
  this->drop_manager->cached_drop_target_container = nullptr;
  this->drop_manager->cached_drop_target_request = nullptr;
  this->drop_manager->cached_drop_target = nullptr;
  this->drop_manager->cached_drop_damage_code = nullptr;

  //set cached pre-drop fields back to nullptr, to ensure a pre-drop image is also taken for every drop target provided
  this->drop_manager->cached_pre_container = nullptr;
  this->drop_manager->cached_pre_request = nullptr;


  Logger::Instance()->Debug("Bag ID in drop target request is: {}", details->bag_id);
  if (details->bag_id.compare(this->bag_id) != 0)
  {
    Logger::Instance()->Error("Drop Target - State Bag ID Mismatch; Vars: state_bag_id = {}; Cam: LFB; PKID: {}",
                              this->bag_id, PrimaryKeyID);
    return std::make_shared<DropResult>(details->request_id,
                                        DropTargetErrorCodes::BagIdMismatch,
                                        "Bag ID in drop target request is: `" + details->bag_id +
                                        "` while Bag ID in the dispense manager is: `" + this->bag_id + "`"); //Todo: move to throw/catch format, log data
  }

  std::shared_ptr<std::string> base_directory = this->create_datagenerator_basedir();

  std::shared_ptr<std::string> time_stamp_string = FileSystemUtil::create_datetime_string();

  if(!this->dispense_reader->GetBoolean("device_specific", "use_advanced_damage_criteria", false))
  {
    Logger::Instance()->Warn("Not using advanced damage criteria, check configs");
    details->item_damage_code = details->item_material;
  }
  else
  {
    Logger::Instance()->Info("Using advanced damage criteria, damage code for this item: {}", details->item_damage_code);
  }

  /**
   *  Call method in live viewer to rename certain visualizations if they already exist
   *  This would occur during an LFB Pirouette - where the drop target + markers images will already exist from previous request
   */

  //call drop target algorithm
  std::shared_ptr<fulfil::dispense::drop::DropResult>
    drop_result = this->drop_manager->handle_drop_request(request_json, details, base_directory, time_stamp_string,
                                                            true, this->bot_already_rotated_for_current_dispense);

  //if algorithm failed with no drop target, upload available visualizations immediately
  if (drop_result->success_code != DropTargetErrorCodes::Success and drop_result->success_code != DropTargetErrorCodes::EmptyBagNotEmpty)
  {
    if (this->live_viewer != nullptr)
    {
      std::shared_ptr<std::vector<std::string>> message = std::make_shared < std::vector <std::string>> ();
      std::string error_line = "Target: Error ID " + std::to_string(drop_result->success_code);
      message->push_back(error_line);
      std::string specific_error_message = get_error_name_from_code((DropTargetErrorCodes) drop_result->success_code);
      message->push_back(specific_error_message);
      std::cout << specific_error_message << std::endl;
      this->live_viewer->update_image(live_viewer->get_blank_visualization(), ViewerImageType::Info, PrimaryKeyID, true, message);

      Logger::Instance()->Debug("Handle Drop Target Failed!");

    }
  }
  else //case for valid drop target result without error
  {
    if (drop_result->Rotate_LFB)
    {
      Logger::Instance()->Info("Drop Target Result recommends to rotate LFB; caching to disallow further rotation on this dispense");
      this->bot_already_rotated_for_current_dispense = true;
    }
  }

  Logger::Instance()->Debug("Finished handling Drop Target Request for Bay: {}", this->machine_name);
  return drop_result;
}

std::shared_ptr<PostLFRResponse> DispenseManager::handle_post_LFR(std::shared_ptr<std::string> PrimaryKeyID,
                                                                  std::shared_ptr<std::string> request_id,
                                                                  std::shared_ptr<nlohmann::json> request_json) {
  auto timer = fulfil::utils::timing::Timer("DispenseManager::handle_post_LFR for " + this->machine_name + " request " + *PrimaryKeyID);
  Logger::Instance()->Debug("Handling Post LFR Command {} for Bay: {}", *PrimaryKeyID,  this->machine_name);
  auto make_bounce_error = [&request_id]() {
    Logger::Instance()->Warn("No LFB Session: Bouncing Drop Camera Post LFR");
    return std::make_shared<PostLFRResponse>(request_id, 12);
  };

    //set cached post_drop fields back to nullptr
  this->drop_manager->cached_post_container = nullptr; //reset to nullptr before processing begins, in case encounter errors and prepostcomparison is not possible
  this->drop_manager->cached_post_request = nullptr;
  this->bot_already_rotated_for_current_dispense = false; //reset this flag because handle_post_dispense indicates a dispense just happened

  std::shared_ptr<std::string> base_directory = this->create_datagenerator_basedir();
  //call post-drop algorithm
  std::shared_ptr<PostLFRResponse> response = (this->LFB_session) ? this->drop_manager->handle_post_LFR(
          request_json,
          base_directory, request_id, true) : make_bounce_error();
  //pre-post compare (if applicable)
  if (response->get_success_code() != DropTargetErrorCodes::Success)
  {
      Logger::Instance()->Warn("Post drop analysis failed, will not do pre/post comparison.");
  }
  else
  {
      int comparison_flag = this->dispense_reader->GetInteger("drop_zone_searcher", "pre_post_comparison", 0);
      if (comparison_flag == 1)
      {
          std::pair<int,int> detection_results = this->drop_manager->handle_pre_post_compare(*PrimaryKeyID);
          response->set_items_dispensed(detection_results);

          //if pre/post comparison failed, there may have been request input issues, we do not do product fit check in this case
          if (detection_results.first != -1){
              std::vector<int> products_to_overflow = this->drop_manager->check_products_for_fit_in_bag(request_json);
              Logger::Instance()->Debug("Bag fit check: found {} products of interest that will no longer fit in this bag", products_to_overflow.size());
              response->set_products_to_overflow(products_to_overflow);
          }
      }
  }

  return response;
}

void DispenseManager::handle_start_lfb_video(std::shared_ptr<std::string> PrimaryKeyID)
{
    std::string base_directory = this->dispense_reader->Get(dispense_reader->get_default_section(), "vid_gen_base_buffer_dir");
    if (!this->LFB_session or !this->lfb_upload_generator) {
      Logger::Instance()->Warn("Bouncing Bay {} Drop Camera Video; Either missing session or generator", this->machine_name);
      return;
    }

    //check that previous LFB video has already been stopped, as expected by standard sequence of requests
    if(!this->lfb_upload_generator->stop_video_saving) // Move check to video uploader
    {
      Logger::Instance()->Warn("Previous Drop Camera video may still be running! Will send stop signal now and wait 250ms before proceeding");
      this->lfb_upload_generator->stop_video_delay = 0;
      this->lfb_upload_generator->stop_video_saving = true;
      usleep(250000);
    }

    // the stop signal must be turned OFF before a new video generator begins
    this->lfb_upload_generator->stop_video_saving = false;

    std::string cloud_directory = make_media::paths::join_as_path(this->store_id, "dispenses").string();
    std::shared_ptr<GCSSender> sender =  std::make_shared<GCSSender>(this->cloud_media_bucket, cloud_directory);

    std::thread lfb_video_recorder_thread([this, PrimaryKeyID, base_directory, sender]()   //start new lfb_video_recorder_thread for saving LFB video
                       {
                           auto remote_video_path = make_media::paths::join_as_path(*PrimaryKeyID, "LFB_Video");
                           auto local_video_path = make_media::paths::join_as_path(base_directory, remote_video_path);
                           Logger::Instance()->Info("Drop Camera Video for {} starting!", *PrimaryKeyID);
                           this->lfb_upload_generator->save_many_frames(this->LFB_session, sender, 60, 5, local_video_path,
                                                                        remote_video_path, false, true);
                           Logger::Instance()->Info("Drop Camera Video for {} Ended!", *PrimaryKeyID);
                       });
    lfb_video_recorder_thread.detach();
}

void DispenseManager::handle_start_tray_video(std::shared_ptr<std::string> PrimaryKeyID)
{
    std::string base_directory = this->dispense_reader->Get(dispense_reader->get_default_section(), "vid_gen_base_buffer_dir");
    if (!this->tray_session or !this->tray_upload_generator) {
      Logger::Instance()->Warn("Bouncing Bay {} Tray Camera Video; Either missing session or generator", this->machine_name);
      return;
    }
    //check that previous tray video has already stopped, as expected by standard sequence of requests
    if(!this->tray_upload_generator->stop_video_saving) // Move check to video uploader
    {
      Logger::Instance()->Warn("Previous Tray video may still be running! Will send stop signal now and wait 250ms before proceeding");
      this->tray_upload_generator->stop_video_delay = 0;
      this->tray_upload_generator->stop_video_saving = true;
      usleep(250000);
    }
    
    // the stop signal must be turned OFF before a new video generator begins
    this->tray_upload_generator->stop_video_saving = false;

    std::string cloud_directory = make_media::paths::join_as_path(this->store_id, "dispenses").string();
    std::shared_ptr<GCSSender> sender =  std::make_shared<GCSSender>(this->cloud_media_bucket, cloud_directory);

    std::thread tray_video_recorder_thread([this, PrimaryKeyID, base_directory, sender]()   //start new thread for saving tray video
                        {
                            auto remote_video_path = make_media::paths::join_as_path(*PrimaryKeyID, "Tray_Video");
                            auto local_video_path = make_media::paths::join_as_path(base_directory, remote_video_path);
                            Logger::Instance()->Info("Tray Camera Video for {} starting!", *PrimaryKeyID);

                            this->tray_upload_generator->save_many_frames(this->tray_session,sender, 60, 5, local_video_path,
                                                                          remote_video_path, false, true);
                            Logger::Instance()->Info("Tray Camera Video for {} Ended!", *PrimaryKeyID);
                        });
    tray_video_recorder_thread.detach();
}


void DispenseManager::handle_stop_lfb_video()
{
    Logger::Instance()->Debug("Stopping LFB Video Generators with delay: 0 ms");
    this->lfb_upload_generator->stop_video_delay = 0;
    this->lfb_upload_generator->stop_video_saving = true;
    // use a signal type instead of sleep
    //TODO: this value should be based on the fps value for the video generator (e.g. 5fps = 200ms, 300ms > 200ms)
    usleep(400000); //sleep for long enough for stop flag to take effect, even with framerate pause in save_many_frames
    Logger::Instance()->Debug("Stopping LFB video handling is returning now");
}

void DispenseManager::handle_stop_tray_video(int delay)
{
    Logger::Instance()->Debug("Stopping Tray Video Generators with delay: {} ms", delay);
    this->tray_upload_generator->stop_video_delay = delay;
    this->tray_upload_generator->stop_video_saving = true;
    //TODO: this value should be based on the fps value for the video generator (e.g. 5fps = 200ms, 300ms > 200ms)
    usleep(400000); //sleep for 0.4s long enough for stop flag to take effect, even with framerate pause in save_many_frames
    Logger::Instance()->Debug("Stopping Tray video handling is returning now");
}

void DispenseManager::handle_stop_request(std::shared_ptr<std::string> command_id)
{
    std::shared_ptr<ProcessingQueuePredicate<std::shared_ptr<DispenseRequest>>> predicate = std::make_shared<DispenseProcessingQueuePredicate>(command_id);
    this->processing_queue->purge_queue(predicate);
}

results_to_vlsg::TrayValidationCounts dispatch_to_count_api(const std::shared_ptr<fulfil::dispense::tray::TrayManager>& tray_manager,
                            std::string& saved_images_base_directory, const request_from_vlsg::TrayRequest& tray_req,
                            std::vector<tray_count_api_comms::LaneCenterLine>& center_pixels, std::vector<bool>& tongue_detections){
    if (center_pixels.empty()) { return results_to_vlsg::TrayValidationCounts{}; }
    try {
        results_to_vlsg::TrayValidationCounts count_response = tray_manager->dispatch_request_to_count_api(tray_req,
                                                                                                           center_pixels, saved_images_base_directory);
        count_response.update_lane_tongue_detections(tongue_detections);
        Logger::Instance()->Trace("Return body from {} count api query:\n\t{}.", tray_req.get_sequence_step(),
                                  nlohmann::json(count_response).dump());
        return count_response;
    } catch(const std::exception & e) {
        Logger::Instance()->Error("Issue getting count response for {} Request: \n\t{}",  tray_req.get_sequence_step(), e.what());
        return results_to_vlsg::TrayValidationCounts{};
    }
}


//template<typename SaveFN, typename SendFN, typename RunFN>
std::shared_ptr<ItemEdgeDistanceResponse>
DispenseManager::handle_item_edge_distance(std::shared_ptr<std::string> command_id, std::shared_ptr<nlohmann::json> request_json)
{

    request_from_vlsg::TrayRequest single_lane_val_req = request_json->get<request_from_vlsg::TrayRequest>();
    auto timer = fulfil::utils::timing::Timer("DispenseManager::handle_item_edge_distance for " + this->machine_name + " request " + single_lane_val_req.m_context.get_id_tagged_sequence_step());
    Logger::Instance()->Debug("Handling {} Dispense Lane Processing {} for Bay: {}", single_lane_val_req.get_sequence_step(), single_lane_val_req.get_primary_key_id(), this->machine_name);

    if (!this->tray_session) {
        Logger::Instance()->Warn("No Tray Session on Bay {}: Bouncing Tray Item Edge Distance!", this->machine_name);
        return std::make_shared<ItemEdgeDistanceResponse>(command_id, 12);
    }

    auto is_pre_dispense = single_lane_val_req.get_sequence_step().at(2) == 'e';
    auto image_code = is_pre_dispense ? ViewerImageType::Tray_Pre_Dispense : ViewerImageType::Tray_Post_Dispense;
    if(this->live_viewer) {
        this->live_viewer->update_image( std::make_shared<cv::Mat>(this->tray_session->grab_color_frame()), image_code, single_lane_val_req.get_primary_key_id(), true);
    }
    auto saved_images_base_directory = this->dispense_reader->Get(this->dispense_reader->get_default_section(), "data_gen_image_base_dir");
    IniSectionReader section_reader {*this->tray_config_reader, this->tray_dimension_type};
    //TrayAlgorithm tray_algorithm = TrayAlgorithm(section_reader);
    Tray tray = this->tray_manager->create_tray(single_lane_val_req.m_tray_recipe);

    /** run algorithms **/
    auto run_fed_processing = [&tray_cam=this->tray_session,
            &section_reader, &tray](request_from_vlsg::TrayRequest& lane_req) {
        try{
            TrayAlgorithm tray_algorithm = TrayAlgorithm(section_reader);
            return tray_algorithm.run_tray_algorithm(tray_cam, lane_req, tray);
        } catch(const std::exception & e) {
            return std::make_tuple(results_to_vlsg::LaneItemDistance{},
                                   std::vector<tray_count_api_comms::LaneCenterLine>{}, std::vector<bool>{});
            //return std::tuple<results_to_vlsg::LaneItemDistance, std::vector<tray_count_api_comms::LaneCenterLine>, std::vector<bool>> {
            //        results_to_vlsg::LaneItemDistance{}, std::vector<tray_count_api_comms::LaneCenterLine>{}, std::vector<bool>{}};
        }
    };

    auto do_all_tray_processing = [&tm=tray_manager, &single_lane_val_req,
                                   &command_id, &saved_images_base_directory](auto fed_process) {
        auto [fed_result, transformed_lane_center_pixels, tongue_detections] = fed_process(single_lane_val_req);
        //results_to_vlsg::TrayValidationCounts count_response = count_dispatch(single_lane_val_req, transformed_lane_center_pixels, tongue_detections);
        results_to_vlsg::TrayValidationCounts count_response = dispatch_to_count_api(tm, saved_images_base_directory,
                                                                  single_lane_val_req, transformed_lane_center_pixels, tongue_detections);
        return ItemEdgeDistanceResponse(fed_result, count_response, command_id);

    };

    this->tray_session->refresh();
    /** Save Data from generator */
    DataGenerator single_lane_tray_data_generator = tray_manager->build_tray_data_generator(
            request_json, tray_manager->make_default_datagen_path(saved_images_base_directory, single_lane_val_req) / single_lane_val_req.get_sequence_step());
    single_lane_tray_data_generator.save_data(std::make_shared<std::string>());

    auto tray_result = std::make_shared<ItemEdgeDistanceResponse>(do_all_tray_processing(run_fed_processing));

    Logger::Instance()->Info("Finished handling Single Lane Dispense command. Result: "
                              "Bay: {} PKID: {} Distance {}", this->machine_name, single_lane_val_req.get_primary_key_id(), tray_result->get_fed_value());
    return  tray_result;
}


//Todo: Should also make a handler class
std::shared_ptr<TrayValidationResponse>
DispenseManager::handle_tray_validation(std::shared_ptr<std::string> command_id, std::shared_ptr<nlohmann::json> request_json)
{
    auto make_null_tray_validation_result = [command_id]() { // by value since it's a fuckin ptr
        auto tray_result = nlohmann::json(results_to_vlsg::TrayValidationCounts{});
        tray_result["Error"] = 12;
        return std::make_shared<TrayValidationResponse>(command_id, std::make_shared<std::string>(tray_result.dump()));
    };
    
    request_from_vlsg::TrayRequest tray_validation_request = request_json->get<request_from_vlsg::TrayRequest>();
    auto timer = fulfil::utils::timing::Timer("DispenseManager::handle_tray_validation for " + this->machine_name + " request " + tray_validation_request.get_primary_key_id());
    Logger::Instance()->Debug("Handling Tray Validation Command {} for Bay: {}", tray_validation_request.get_primary_key_id(), this->machine_name);
    
    if (!this->tray_session) {
        Logger::Instance()->Warn("No Tray Session: Bouncing Tray Validation Request");
        return make_null_tray_validation_result();
    }

    auto saved_images_base_directory = this->dispense_reader->Get(this->dispense_reader->get_default_section(), "data_gen_image_base_dir");
    double resize_factor = 1;
    /** Send Data Function */
    std::string local_base_path = this->dispense_reader->Get(dispense_reader->get_default_section(), "upload_file_dir", "");

    auto send_color_mat = [&] (double scale_resize) {
        try {
            if (this->tray_manager->save_tray_audit_image(tray_validation_request.m_context, local_base_path, scale_resize)) {
              return this->tray_manager->upload_tray_data(tray_validation_request.m_context, local_base_path, GCSSender(this->cloud_media_bucket, this->store_id));
            }
        } catch(const std::exception & e) {
            Logger::Instance()->Error("Caught exception in tvr audit upload: \n\t{}", e.what());
        }
        return -1;
    };

    /** Save Data from generator */
    DataGenerator tray_validation_data_generator = tray_manager->build_tray_data_generator(
      request_json, tray_manager->make_default_datagen_path(saved_images_base_directory, tray_validation_request) / tray_validation_request.get_sequence_step());
    auto save_data_fn = [&tray_validation_data_generator, seq_step=tray_validation_request.get_sequence_step()]() {
        tray_validation_data_generator.save_data(std::make_shared<std::string>());
    };


    IniSectionReader section_reader {*this->tray_config_reader, this->tray_dimension_type};
    std::shared_ptr<TrayAlgorithm> tray_algorithm = std::make_shared<TrayAlgorithm>(section_reader);
    Tray tray = this->tray_manager->create_tray(tray_validation_request.m_tray_recipe);

    /** run algorithms **/
    auto dispatch_to_count_api = [&tray_validation_request, &saved_images_base_directory, tm=tray_manager](auto center_pixels) {
        return tm->dispatch_request_to_count_api(tray_validation_request, center_pixels, saved_images_base_directory);
    };

    auto do_all_tray_processing = [&](auto save_function, auto run_function) {
      try {
        save_function();
        auto [transformed_lane_center_pixels, tongue_detections, max_height_detected_in_tray] =
            tray_algorithm->get_pixel_lane_centers_and_tongue_detections(
                this->tray_session, tray_validation_request, tray);
        results_to_vlsg::TrayValidationCounts count_response = run_function(transformed_lane_center_pixels);
        count_response.update_lane_tongue_detections(tongue_detections);
        auto exp_max = tray_validation_request.expected_max_height();
        count_response.m_height_info = results_to_vlsg::TrayHeight(max_height_detected_in_tray, exp_max);
        float detected_expected_height_diff = max_height_detected_in_tray - exp_max;
        float error_over_exp = max_height_detected_in_tray/exp_max;
        Logger::Instance()->Info("Request {} has tallest item tray buffer of {}, and returned height of tray is {} with the confidence {}."
            " After adjusting for tray inset, error is {} and detected height is {:0.4f} of expected height.",
            tray_validation_request.get_primary_key_id(),  exp_max, max_height_detected_in_tray, count_response.m_height_info.m_confidence,
            detected_expected_height_diff, error_over_exp);
        Logger::Instance()->Info("Return body from Tray Validation count api query:\n\t{}.", nlohmann::json(count_response).dump());
        return count_response;
      } catch(const std::exception & e) {
        Logger::Instance()->Error("Issue saving or uploading data from Tray Validation Request: \n\t{}", e.what());
      }
      Logger::Instance()->Info("Sending nominal response in Tray Validation Request");
      return results_to_vlsg::TrayValidationCounts{};
    };

    if (send_color_mat(resize_factor) < 0) {
        Logger::Instance()->Error("Issue uploading Tray Validation audit data for {}", tray_validation_request.get_primary_key_id());
    };
    
    this->tray_session->refresh();
    auto tray_result = nlohmann::json(do_all_tray_processing(save_data_fn, dispatch_to_count_api));


    tray_result["Error"] = 0;
    std::shared_ptr<std::string> payload = std::make_shared<std::string>(tray_result.dump());
    Logger::Instance()->Debug("Finished handling Tray Validation Command for Bay: {} PKID: {} Json response:\n\t{}",
                              this->machine_name, tray_validation_request.get_primary_key_id(), *payload);
    return std::make_shared<TrayValidationResponse>(command_id, payload);
}

int DispenseManager::handle_update_state(std::shared_ptr<std::string> PrimaryKeyID, std::shared_ptr<nlohmann::json> request_json)
{
  Logger::Instance()->Trace("Handle Get State called in Dispense Manager");
  if (!this->LFB_session) {
    Logger::Instance()->Warn("No LFB Session: Bouncing Drop Camera Get State");
    return 12;
  }

  this->bot_already_rotated_for_current_dispense = false; //get state indicates a new bot, reset this flag here
  try {

    auto bag = nlohmann::to_string((*request_json)["Bag_State"]);
    std::cout << "CvBagState JSON from FC: " << bag << std::endl;
    auto cvbag = fulfil::mongo::CvBagState((*request_json)["Bag_State"]);
    this->drop_manager->mongo_bag_state->parse_in_values((std::make_shared<fulfil::mongo::CvBagState>(cvbag)));
    this->bag_id = this->drop_manager->mongo_bag_state->raw_mongo_doc->BagId;
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

std::string DispenseManager::handle_get_state(std::shared_ptr<std::string> PrimaryKeyID, std::shared_ptr<nlohmann::json> request_json)
{
    Logger::Instance()->Trace("Handle Update State called in Dispense Manager");
    try
    {
        if(this->drop_manager->mongo_bag_state->raw_mongo_doc == nullptr)this->drop_manager->mongo_bag_state->raw_mongo_doc = std::make_shared<fulfil::mongo::CvBagState>();
        
        return this->drop_manager->mongo_bag_state->GetStateAsString();

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

int DispenseManager::handle_pre_LFR(std::shared_ptr<std::string> PrimaryKeyID,
                                    std::shared_ptr<nlohmann::json> request_json)
{
    auto timer = fulfil::utils::timing::Timer("DispenseManager::handle_pre_LFR for " + this->machine_name + " request " + *PrimaryKeyID);
    Logger::Instance()->Debug("Handling Pre Drop Command {} for Bay: {}", *PrimaryKeyID, this->machine_name);
    

    if (!this->LFB_session) {
      Logger::Instance()->Warn("No LFB Session: Bouncing Drop Camera Pre LFR");
      return 12;
    }

    try
    {
        float remaining_platform = (*request_json)["Remaining_Platform"].get<float>();
        Logger::Instance()->Debug("Remaining_Platform: {}", remaining_platform);

        //this->LFB_session->set_emitter(true); //turn on emitter for imaging
        this->LFB_session->refresh();
        //this->LFB_session->set_emitter(false); //turn off emitter after imaging

        Logger::Instance()->Debug("Getting container for caching now");
        std::shared_ptr<MockSession> mock_session_pre = std::make_shared<MockSession>(this->LFB_session);
        std::shared_ptr<MarkerDetectorContainer> container = this->drop_manager->searcher->get_container(this->drop_manager->mongo_bag_state->raw_mongo_doc->Config,
                                                                                                         mock_session_pre,
                                                                                                         this->drop_manager->mongo_bag_state->raw_mongo_doc->Config->extend_depth_analysis_over_markers);

        this->drop_manager->cached_pre_container = container; //cache for potential use in prepostcomparison later
        this->drop_manager->cached_pre_request = request_json; //cache for potential use in prepostcomparison later

        /**
         *  Send images to Live Viewer
         */
        if (this->live_viewer != nullptr)
        {
            this->live_viewer->update_image(this->LFB_session->get_color_mat(), ViewerImageType::LFB_Pre_Dispense, *PrimaryKeyID);
        }

        /**
         *  Data generation. Todo: refactor to reduce total code (mostly repeated code in drop_manager)
         */
        std::shared_ptr<std::string> base_directory = this->create_datagenerator_basedir();

        std::shared_ptr<std::string> time_stamp = FileSystemUtil::create_datetime_string();

        FileSystemUtil::join_append(base_directory, "Drop_Camera");
        FileSystemUtil::join_append(base_directory, *PrimaryKeyID);
        FileSystemUtil::join_append(base_directory, "Pre_Drop_Image");

        // Setting up error code file saving
        std::shared_ptr<std::string> error_code_file = std::make_shared<std::string>(*base_directory);
        FileSystemUtil::join_append(error_code_file, *time_stamp);
        FileSystemUtil::join_append(error_code_file, "error_code");

        Logger::Instance()->Trace("Base directory is {}", *base_directory);
        std::shared_ptr<DataGenerator> generator = std::make_shared<DataGenerator>(this->LFB_session,
                                                                                   base_directory,
                                                                                   request_json);
        generator->save_data(time_stamp);

        Logger::Instance()->Trace("Saving error code file with code 0 at path {}", *error_code_file);
        std::ofstream error_file(*error_code_file);
        error_file << "0";

        return 0;
    }
    catch(const std::exception & e)
    {
        Logger::Instance()->Error("Issue handling pre dispense request: \n\t{}", e.what());

        return 1;
    }
    catch(...)
    {
        Logger::Instance()->Error("Unspecified error caught during pre dispense request handling");

        return 1;
    }
}

int DispenseManager::handle_home_motor(std::shared_ptr<std::string> PrimaryKeyID, std::shared_ptr<nlohmann::json> request_json)
{

    if (!this->LFB_session) {
      Logger::Instance()->Warn("No LFB Session: Bouncing Home Motor");
      return 12;
    }

  if(motion_controller == nullptr)
  {
    Logger::Instance()->Error("Received motion request but motor not configured to run;");
    exit(15);
  }

  if(!motor_in_position) //there are scenarios where a VLSG high command fails while the motor is homing or positioning, and a new home request may happen
  {
    Logger::Instance()->Error("Received motor command while motor already in motion;");
    Logger::Instance()->Info("sending motor interrupt signal to stop any currently running threads and waiting 1000ms before continuing");
    this->motor_motion_interrupted = true;
    usleep(1000000);
  }

  Logger::Instance()->Info("Sending Stop Motor Command now before sending new homing command");
  motion_controller->StopMotor();
  usleep(200000);
  this->motor_motion_interrupted = false;
  motor_in_position = false;

  bool initial_command_check = motion_controller->SendMotorCommand(0x02,TMCL_ROL, 0, 0, home_velocity_setting); //Rotate left, value in pps (= microsteps/sec))
  if(!initial_command_check)
  {
    Logger::Instance()->Error("Motion command failed; Vars: home_velocity_command;");
    exit(16);
  }

  std::thread home_motor_thread([this]()
  {
    int max_sensor_read_value = -1000; //for tracking what the max detected analog sensor input is, during this homing routine
    int max_rotation_time_seconds = 20; //seconds
    float sleep_time_ms = 100; //milliseconds
    int remaining_loops = (max_rotation_time_seconds * 1000) / sleep_time_ms;
    int sensor_threshold = motion_config_reader.GetInteger("motion_parameters", "sensor_threshold", -1);;
    Logger::Instance()->Info("Homing routine in separate thread, for a total of {} seconds, thresh: {}", max_rotation_time_seconds, sensor_threshold);
    bool successfully_homed = false;

    while(remaining_loops > 0)
    {
      if(this->motor_motion_interrupted)
      {
        Logger::Instance()->Warn("INTERRUPT DETECTED in home motor thread, breaking and stopping motor now");
        break;
      }
      int analog_input = motion_controller->GetAnalogSensorInput();
      if(analog_input > max_sensor_read_value) max_sensor_read_value = analog_input; //update max sensor reading for logging purposes
      if(analog_input > sensor_threshold)
      {
        Logger::Instance()->Info("Analog sensor has been triggered with value: {}, stopping motor now!", analog_input);
        successfully_homed = true;
        break;
      }
      Logger::Instance()->Debug("Analog input reads: {}", analog_input);
      usleep(sleep_time_ms * 1000);
      remaining_loops--;
    }

    motion_controller->StopMotor();
    usleep(200000);
    Logger::Instance()->Info("Max sensor reading during homing routine: {}", max_sensor_read_value);

    if(!successfully_homed)
    {
      if(!this->motor_motion_interrupted) Logger::Instance()->Error("Motion command failed; Vars: home_destination_timeout;");
      if(this->motor_motion_interrupted) Logger::Instance()->Error("Motion command failed; Vars: motor_motion_interrupt;");
      return;
    }

    Logger::Instance()->Info("Setting motor absolute position to 0", max_sensor_read_value);
    if(!motion_controller->SendMotorCommand(0x02, TMCL_SAP, MOT_ActualPosition, 0, 0))
    {
      Logger::Instance()->Error("Motion command failed; Vars: zero_position_value;");
      exit(16);
    }
    motion_controller->update_and_print_motor_status();
    motor_in_position = true;
  });
  home_motor_thread.detach();

  //TODO: handle error cases --> what if sensor never goes high, or doesn't home in allowed time frame. Can set error codes via NOP command as well (or timeout on VLSG side)
  Logger::Instance()->Info("handle home motor request returning now");
  return 0;
}


int DispenseManager::handle_position_motor(std::shared_ptr<std::string> PrimaryKeyID, std::shared_ptr<nlohmann::json> request)
{
    if (!this->LFB_session) {
      Logger::Instance()->Warn("No LFB Session: Bouncing Position Motor");
      return 12;
    }

  if(motion_controller == nullptr)
  {
    Logger::Instance()->Error("Received motion request but motor not configured to run");
    exit(15);
  }

  if(!motor_in_position) //there are scenarios where a VLSG high command fails while the motor is homing or positioning, and a new home request may happen
  {
    Logger::Instance()->Error("Received position motor command while motor already in motion;");
    Logger::Instance()->Info("sending motor interrupt signal to stop currently running threads and waiting 1000ms before continuing");
    this->motor_motion_interrupted = true;
    usleep(1000000);
  }

  Logger::Instance()->Info("Sending Stop Motor Command now before sending new position command");
  motion_controller->StopMotor();
  usleep(200000);
  this->motor_motion_interrupted = false;
  motor_in_position = false;

  //TODO: add system diagram for how the coordinates work. Position is relative to the homing position for tray which is on the RIGHT side as you are facing the tray from outside the VLS, so positions will be NEGATIVE values
  int target_position_relative_tray_zero = (*request)["Target_Position"].get<int>();
  int trinamic_home_offset_from_tray_zero = motion_config_reader.GetInteger("motion_parameters", "trinamic_home_offset_from_tray_zero", -1);
  float position_conversion_pulses_per_mm = motion_config_reader.GetFloat("motion_parameters", "position_conversion_pulses_per_mm", -1);

  if(trinamic_home_offset_from_tray_zero == -1 or position_conversion_pulses_per_mm == -1)
  {
    Logger::Instance()->Error("Invalid config for motor position motion;", target_position_relative_tray_zero);
    exit(16);
  }
  else if(abs(target_position_relative_tray_zero) > 1500)
  {
    Logger::Instance()->Error("Invalid Motor Position Request; Vars: target_position_relative_to_tray_zero: {}", target_position_relative_tray_zero);
    return -1;
  }
  int target_position_from_trinamic_home = target_position_relative_tray_zero - trinamic_home_offset_from_tray_zero; //in mm
  int target_position = position_conversion_pulses_per_mm * target_position_from_trinamic_home; // in pps relative to trinamic home position

  Logger::Instance()->Info("Requested to send motor to: {} mm from tray zero = {} mm from motor zero = {} position in pps", target_position_relative_tray_zero,
                           target_position_from_trinamic_home, target_position);
  int upper_travel_limit = motion_config_reader.GetInteger("motion_parameters", "upper_travel_limit", 100);
  Logger::Instance()->Debug("Upper travel limit from trinamic home is: {} mm", upper_travel_limit);

  // prevent rail motion from going too far
  if(target_position_from_trinamic_home > upper_travel_limit and ((target_position_from_trinamic_home - upper_travel_limit) < 100))
  {
    Logger::Instance()->Warn("Target position is above upper limit and within 100mm of upper limit, modifying to upper limit right now");
    target_position_from_trinamic_home = upper_travel_limit;
    target_position = position_conversion_pulses_per_mm * target_position_from_trinamic_home; // in pps relative to trinamic home position
  }

  if(target_position_from_trinamic_home < 0 or target_position_from_trinamic_home > (upper_travel_limit)) //sanity check in mm from trinamic home
  {
    Logger::Instance()->Error("Invalid Motor Position Request; Vars: target_position_from_trinamic_home: {}", target_position_from_trinamic_home);
    exit(16);
  }

  Logger::Instance()->Info("Sending position move request to Trinamic now, target in microsteps: {}", target_position);
  if(!motion_controller->SendMotorCommand(0x02, TMCL_MVP, MVP_ABS, 0, target_position)) //absolute positioning move
  {
    Logger::Instance()->Error("Motion command failed; Vars: absolute_position_command;");
    exit(16);
  }

  std::thread home_motor_thread([this, target_position]()
  {
    bool successfully_in_position = false;
    int remaining_loops = 100; //each loop will take ~200 ms. Total motion time: 20 seconds
    while(remaining_loops > 0)
    {
      if(this->motor_motion_interrupted)
      {
        Logger::Instance()->Warn("INTERRUPT DETECTED in position motor thread, breaking and stopping motor now");
        break;
      }

      int reached_target = motion_controller->check_position_reached();
      if(reached_target)
      {
        Logger::Instance()->Info("Motor has reached target position! Will wait briefly before returning to add settling time");
        successfully_in_position = true;
        break;
      }
      int current_position = motion_controller->get_current_position();
      Logger::Instance()->Debug("POSITION MOTOR THREAD: Current motor position in microsteps is: {}", current_position);
      usleep(200000);
      remaining_loops--;
    }

    motion_controller->StopMotor();
    usleep(200000);

    if(!successfully_in_position)
    {
      if(!this->motor_motion_interrupted) Logger::Instance()->Error("Motion command failed; Vars: position_timeout;");
      if(this->motor_motion_interrupted) Logger::Instance()->Error("Motion command failed; Vars: motor_motion_interrupt;");
      return;
    }
    motor_in_position = true; //variable which updates the NOP response for indicating to VLSG that motor has completed motion
  });
  home_motor_thread.detach();
  return 0;
}

bool DispenseManager::check_motor_in_position()
{
  return motor_in_position;
}

std::shared_ptr<fulfil::dispense::bays::BayRunner> fulfil::dispense::DispenseManager::get_shared_ptr()
{
    return this->shared_from_this();
}

std::shared_ptr<std::string> fulfil::dispense::DispenseManager::create_datagenerator_basedir() {
    std::shared_ptr<std::string> base_directory = std::make_shared<std::string>(this->dispense_reader->Get(dispense_reader->get_default_section(), "data_gen_image_base_dir", ""));
    if (base_directory->length() == 0) {
        Logger::Instance()->Fatal("Could not find data_gen_image_base_dir value in section {} of main_config.ini!", dispense_reader->get_default_section());
        throw std::runtime_error("Issue getting config settings.");
    }
    if (base_directory->back() == '/') base_directory->pop_back();
    base_directory->append("_").append(*FileSystemUtil::create_datetime_string(true));

    return base_directory;
}
