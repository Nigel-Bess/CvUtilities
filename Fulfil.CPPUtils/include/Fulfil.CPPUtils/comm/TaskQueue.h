#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <queue>
#include <chrono>

#include "Fulfil.CPPUtils/comm/depthCams.pb.h"
#include "Fulfil.CPPUtils/comm/depthCams.grpc.pb.h"

#include <Fulfil.CPPUtils/logging.h>

using fulfil::utils::Logger;
using namespace DepthCameras;

inline std::string GetTxObjectIdString(){
    static uint32_t _transaction = 1000;
    static char dt[128];
    auto now = std::chrono::steady_clock::now();
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    std::chrono::seconds s = std::chrono::duration_cast<std::chrono::seconds>(ms);
    std::time_t t = s.count();
    uint16_t frac_sec = ms.count() % 1000;

    struct tm * info;
    info = localtime(&t);
    sprintf(dt, "%02d-%02dT%02d:%02d:%02d.%03u-%u", 
        info->tm_mon +1, info->tm_mday, info->tm_hour, info->tm_min, info->tm_sec, frac_sec, _transaction++); 
    std::string id(dt);
    return id;
}

class TaskQueue final{
public:
    void Clear(){
        std::queue<std::shared_ptr<DcRequest>> emptyq;
        std::deque<std::shared_ptr<DcResponse>> emptyd;
        swap(_requests, emptyq);
        swap(_responses, emptyd);
    }

    bool HasReads(){
        return !_requests.empty(); 
    }

    void PackRead(DcRequest rx){
        auto msg = std::make_shared<DcRequest>(rx);
        if(msg->type() != MESSAGE_TYPE_HEARTBEAT)
            fulfil::utils::Logger::Instance()->Info("---grpc--> {} type {}", msg->command_id(), DepthCameras::MessageType_Name(msg->type()));
        switch(msg->type()){
            case MESSAGE_TYPE_UNSPECIFIED:
                switch(msg->dc_cmd_type()){
                    case DC_COMMAND_UNSPECIFIED:
                        fulfil::utils::Logger::Instance()->Info("Received stream break lock command");//don't ack
                    return;
                    default://handle in mars base

                    break;
                }
            break;
            case MESSAGE_TYPE_HEARTBEAT:
                Acknowledge(*msg);
                return;
            case MESSAGE_TYPE_ACK_REPLY://Don't reply to ACK message!
            return;
            default://handle in mars machine
            break;
        }
        Acknowledge(*msg);
        std::lock_guard<std::mutex> lock(read_mu_);
        _requests.push(msg);
    }

    std::shared_ptr<DcRequest> GetNextRequest(){
        std::lock_guard<std::mutex> lock(read_mu_);
        auto msg = _requests.front();
        //debug("Handling _requests id: %d type %X\n", msg->transaction(), msg->type());
        _requests.pop();
        return msg;
    }

    int WritesLeft(){
        return _responses.size(); 
    }

    DcResponse GetNextWrite(){
        std::lock_guard<std::mutex> lock(write_mu_);
        auto msg = *_responses.front().get();
        //debug("<--grpc--- %s type %02X, AAPI type: %X\n", msg.id().c_str(), msg.type(), msg.aapi_type());
        _responses.pop_front();
        return msg;
    }

    void PackWrite(std::shared_ptr<DcResponse> msg){
        std::lock_guard<std::mutex> lock(write_mu_);
        _responses.push_back(msg);
    }
    
    bool HasNewStatus(){
        std::lock_guard<std::mutex> lock(status_mu_);
        return !_statusUpdates.empty();
    }

    void CreateStatusMessage(std::string id, DcStatusCodes code, std::string function){
        std::string msg;
        CommandStatusUpdate cmd = {};
        cmd.set_msg_type(MESSAGE_TYPE_COMMAND_STATUS);
        cmd.set_command_id(id);
        cmd.set_status_code(code);
        cmd.SerializeToString(&msg);
        fulfil::utils::Logger::Instance()->Info("Status update for %s [%s: %s]\n",function.c_str(), id.c_str(), DcStatusCodes_Name(code).c_str());
        AddStatusUpdate(MESSAGE_TYPE_COMMAND_STATUS, msg, id);
    }

    void AddStatusUpdate(DepthCameras::MessageType t, std::string str, std::string id){
        DcResponse resp = {};
        resp.set_type(t);
        resp.set_command_id(id);
        resp.set_message_data(str.data(), str.size());
        resp.set_message_size(str.size());
        std::lock_guard<std::mutex> lock(status_mu_);
        _statusUpdates.push(std::make_shared<DcResponse>(resp));
    }

    std::shared_ptr<DcResponse> GetStatusMessage(){
        std::lock_guard<std::mutex> lock(status_mu_);
        auto status = _statusUpdates.front();
        _statusUpdates.pop();
        return status;
    }

private:
    void Acknowledge(DcRequest rxMsg){
        DcResponse msg = {};
        msg.set_command_id(rxMsg.command_id());
        msg.set_type(MESSAGE_TYPE_ACK_REPLY);
        DepthCameras::Acknowledge cmd;
        cmd.set_type(MESSAGE_TYPE_ACK_REPLY);
        cmd.set_acknowledged_type(rxMsg.type());
        cmd.set_command_id(rxMsg.command_id());
        char array[cmd.ByteSizeLong()];
        cmd.SerializeToArray((void *)&array, cmd.ByteSizeLong());
        msg.set_message_data((void *)&array, cmd.ByteSizeLong());
        msg.set_message_size(cmd.ByteSizeLong());
        std::lock_guard<std::mutex> lock(write_mu_);
        _responses.push_front(std::make_shared<DcResponse>(msg));
        
        //debug("ACK id: %s type %02X, AAPI type: %X\n", rxMsg.id().c_str(), rxMsg.type(), rxMsg.aapi_type());

    }
    std::mutex read_mu_;
    std::mutex write_mu_;
    std::mutex status_mu_;
    
    std::queue<std::shared_ptr<DcRequest>> _requests; 
    std::deque<std::shared_ptr<DcResponse>> _responses;  
    std::queue<std::shared_ptr<DcResponse>> _statusUpdates; 
};