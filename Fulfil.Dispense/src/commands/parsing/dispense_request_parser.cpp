//
// Created by nkaffine on 12/11/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#include "Fulfil.Dispense/commands/parsing/dispense_request_parser.h"
#include <json.hpp>
#include <Fulfil.Dispense/commands/parsing/dispense_command_parser.h>
#include "Fulfil.Dispense/commands/nop/nop_request.h"
#include <Fulfil.Dispense/commands/get_state_request.h>
#include <Fulfil.Dispense/commands/update_state_request.h>
#include <Fulfil.Dispense/commands/drop_target/drop_target_request.h>
#include <Fulfil.Dispense/commands/item_edge_distance/item_edge_distance_request.h>
#include <Fulfil.Dispense/commands/pre_LFR_request.h>
#include <Fulfil.Dispense/commands/post_drop/post_LFR_request.h>
#include <Fulfil.Dispense/commands/parsing/command_parsing_errors.h>
#include <Fulfil.Dispense/commands/start_tray_video_request.h>
#include <Fulfil.Dispense/commands/start_lfb_video_request.h>
#include <Fulfil.Dispense/commands/stop_tray_video_request.h>
#include <Fulfil.Dispense/commands/stop_lfb_video_request.h>
#include <Fulfil.Dispense/commands/tray_validation/tray_validation_request.h>
#include <Fulfil.Dispense/commands/home_motor_request.h>
#include <Fulfil.Dispense/commands/position_motor_request.h>
#include <Fulfil.CPPUtils/logging.h>
#include <FulfilMongoCpp/mongo_objects/mongo_object_id.h>

using fulfil::utils::Logger;

using fulfil::dispense::commands::DispenseRequestParser;
using fulfil::dispense::commands::DispenseRequestJsonParser;
using fulfil::dispense::commands::DispenseRequest;
using fulfil::dispense::commands::DispenseResponse;
using fulfil::dispense::commands::NopRequest;
using fulfil::dispense::commands::GetStateRequest;
using fulfil::dispense::commands::DropTargetRequest;
using fulfil::dispense::commands::ItemEdgeDistanceRequest;
using fulfil::dispense::commands::PreLFRRequest;
using fulfil::dispense::commands::UpdateStateRequest;
using fulfil::dispense::commands::PostLFRRequest;
using fulfil::dispense::commands::StartLFBVideoRequest;
using fulfil::dispense::commands::StopLFBVideoRequest;
using fulfil::dispense::commands::StartTrayVideoRequest;
using fulfil::dispense::commands::StopTrayVideoRequest;
using fulfil::dispense::commands::TrayValidationRequest;
using fulfil::dispense::commands::errors::InvalidCommandFormatException;

DispenseRequestParser::DispenseRequestParser(std::shared_ptr<DispenseRequestJsonParser> parser)
{
  this->json_parser = parser;
  this->nop_counter = 0;
}

std::shared_ptr<DispenseRequest> DispenseRequestParser::parse_payload(std::shared_ptr<std::string> payload,std::shared_ptr<std::string> request_id){
  std::shared_ptr<nlohmann::json> request_json = std::make_shared<nlohmann::json>(nlohmann::json::parse(payload->c_str()));

  std::cout << "payload---> " << payload.get()->c_str() << std::endl;
  int type = (*request_json)["Type"].get<int>();
  if (type > (int)DispenseCommand::send_bag_state || type < 0)  //check that type is within expected bounds
  {
    Logger::Instance()->Error("Invalid dispense request type, outside of expected bounds");
    throw std::invalid_argument("Invalid Command Type");
  }

  std::shared_ptr<std::string> PrimaryKeyID =  std::make_shared<std::string>((*request_json)["Primary_Key_ID"].get<std::string>());

  std::string request_id_string = bsoncxx::oid((*request_id).c_str()).to_string();
  switch (static_cast<DispenseCommand>(type))
  {
    case DispenseCommand::nop:
      this->nop_counter+= 1;
      if(this->nop_counter >= 1000)
      {
        Logger::Instance()->Trace("A series of 1000 heartbeat requests have been processed");
        this->nop_counter = 0;
      }
      return std::make_shared<NopRequest>(request_id, PrimaryKeyID);
    case DispenseCommand::request_bag_state:
    case DispenseCommand::get_state:
      Logger::Instance()->Info("Received Get State Request, PKID: {}, request_id: {}", *PrimaryKeyID, request_id_string);
      return std::make_shared<GetStateRequest>(request_id, PrimaryKeyID, request_json);
    case DispenseCommand::drop_target:
      Logger::Instance()->Info("Received Drop Target Request, PKID: {}, request_id: {}", *PrimaryKeyID, request_id_string);
      return std::make_shared<DropTargetRequest>(request_id, PrimaryKeyID, std::make_shared<DropTargetDetails>(request_json, request_id), request_json);
    case DispenseCommand::post_LFR:
      Logger::Instance()->Info("Received Post Drop Request, PKID: {}, request_id: {}", *PrimaryKeyID, request_id_string);
      return std::make_shared<PostLFRRequest>(request_id, PrimaryKeyID, request_json);
    case DispenseCommand::start_lfb_video:
      Logger::Instance()->Info("Received Start LFB Video Request, PKID: {}, request_id: {}", *PrimaryKeyID, request_id_string);
      return std::make_shared<StartLFBVideoRequest>(request_id, PrimaryKeyID);
    case DispenseCommand::stop_lfb_video:
      Logger::Instance()->Info("Received Stop LFB Video Request, PKID: {}, request_id: {}", *PrimaryKeyID, request_id_string);
      return std::make_shared<StopLFBVideoRequest>(request_id, PrimaryKeyID);
    case DispenseCommand::start_tray_video:
      Logger::Instance()->Info("Received Start Tray Video Request, PKID: {}, request_id: {}", *PrimaryKeyID, request_id_string);
      return std::make_shared<StartTrayVideoRequest>(request_id, PrimaryKeyID);
    case DispenseCommand::stop_tray_video:
      Logger::Instance()->Info("Received Stop Tray Video Request, PKID: {}, request_id: {}", *PrimaryKeyID, request_id_string);
      return std::make_shared<StopTrayVideoRequest>(request_id, PrimaryKeyID, request_json);
    case DispenseCommand::tray_validation:
      Logger::Instance()->Info("Received Tray Validation Request, PKID: {}, request_id: {}", *PrimaryKeyID, request_id_string);
      return std::make_shared<TrayValidationRequest>(request_id, PrimaryKeyID, request_json);
    case DispenseCommand::item_edge_distance:
      Logger::Instance()->Info("Received Tray Dispense Lane Request, PKID: {}, request_id: {}", *PrimaryKeyID, request_id_string);
      return std::make_shared<ItemEdgeDistanceRequest>(request_id, PrimaryKeyID, request_json);
    case DispenseCommand::pre_LFR:
      Logger::Instance()->Info("Received Pre Drop LFB Request, PKID: {}, request_id: {}", *PrimaryKeyID, request_id_string);
      return std::make_shared<PreLFRRequest>(request_id, PrimaryKeyID, request_json);
    case DispenseCommand::send_bag_state:
    case DispenseCommand::update_state:
      Logger::Instance()->Info("Received Update State Request, PKID: {}, request_id: {}", *PrimaryKeyID, request_id_string);
      return std::make_shared<UpdateStateRequest>(request_id, PrimaryKeyID, request_json);
    case DispenseCommand::home_motor:
      Logger::Instance()->Info("Received Home Motor Request, PKID: {}", *PrimaryKeyID);
      return std::make_shared<HomeMotorRequest>(request_id, PrimaryKeyID, request_json);
    case DispenseCommand::position_motor:
      Logger::Instance()->Info("Received Position Motor Request, PKID: {}", *PrimaryKeyID);
      return std::make_shared<PositionMotorRequest>(request_id, PrimaryKeyID, request_json);

    default:
      throw InvalidCommandFormatException(request_id);
  }
}