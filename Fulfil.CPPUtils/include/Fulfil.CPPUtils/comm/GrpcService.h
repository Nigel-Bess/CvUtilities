#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <queue>

#include <semaphore.h>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/server_context.h>

#include "depthCams.pb.h"
#include "depthCams.grpc.pb.h"
#include "TaskQueue.h"

#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.CPPUtils/timer.h>

using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerReaderWriter;
using grpc::ServerBuilder;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerAsyncReaderWriter;
using DepthCameras::DcApiService;

using fulfil::utils::Logger;
using namespace std::chrono;
using namespace DepthCameras;

enum ChannelStatus
{
    CREATE,
    READ,
    RESPONSE,
    WRITE,
    FINISH
};

#define GRPC_PORT 9501

inline std::string GetTxObjectIdString(){
    static uint32_t _transaction = 1000;
    static char dt[128];
    auto now = system_clock::now();
    milliseconds ms = duration_cast<milliseconds>(now.time_since_epoch());
    seconds s = duration_cast<seconds>(ms);
    std::time_t t = s.count();
    uint16_t frac_sec = ms.count() % 1000;

    struct tm * info;
    info = localtime(&t);
    sprintf(dt, "%02d-%02dT%02d:%02d:%02d.%03u-%u", 
        info->tm_mon +1, info->tm_mday, info->tm_hour, info->tm_min, info->tm_sec, frac_sec, _transaction++); 
    std::string id(dt);
    return id;
}

class AsyncConnection{
    public:     
        AsyncConnection(DcApiService::AsyncService *aservice, ServerCompletionQueue *scq, TaskQueue * task)
                : aservice_(aservice), cq_(scq), tasks_(task), status_(CREATE){}

        virtual void Process() = 0;
        virtual ~AsyncConnection(){}
        DcApiService::AsyncService *aservice_;
        ServerCompletionQueue *cq_;
        TaskQueue *tasks_;
        ServerContext ctx_;
        ChannelStatus status_; // The current serving state.
        bool Connected = false;

};

class DepthCamServiceImpl: public AsyncConnection{
    public:
        DepthCamServiceImpl(DcApiService::AsyncService *aservice, ServerCompletionQueue *scq, TaskQueue * task)
            :AsyncConnection(aservice, scq, task), stream_(&ctx_){
                    Process();
                }
        void Process() override;
        void SendMessage(std::shared_ptr<DcResponse> tx);
        void QueueHelloMessage();
    private:
        std::chrono::system_clock::time_point conn_time_;
        ServerAsyncReaderWriter<DcResponse, DcRequest> stream_; //Type<W, R>
        DcRequest request_ = {};
        DcResponse response_ = {};
        inline void CheckConnection(){
            auto ip = ctx_.peer();
            if(!Connected){
                Logger::Instance()->Info("A gRPC client has connected at {}!", ip);            
            }
            Connected = true;
        }

};


class GrpcService final
{
public:
    void Run(uint16_t port);
    
    void AddStatusUpdate(std::string msg){
        std::lock_guard<std::mutex> lock(_lock);
        _statusQueue.push(msg);
    }

    void QueueResponse(std::shared_ptr<DcResponse> msg){
        tasks_.PackWrite(msg);
    }

    bool HasNewStatus(){
        return !_statusQueue.empty();
    }

    std::string GetStatusMessage(){
        std::lock_guard<std::mutex> lock(_lock);
        auto status = _statusQueue.front();
        _statusQueue.pop();
        return status;
    } 

    bool HasNewRequest(){
        return tasks_.HasReads();
    }

    std::shared_ptr<DcRequest> GetNextRequest(){
        return tasks_.GetNextRequest();
    }

private:
    void HandleRawRpcs();
    void HandleNewMessages();
    std::queue<std::string> _statusQueue;    
    std::mutex _lock;
    TaskQueue tasks_;
    sem_t _cmd_wait;
    std::unique_ptr<ServerCompletionQueue> cq_;
    DcApiService::AsyncService async_service_;
    std::unique_ptr<Server> server_;
    DepthCamServiceImpl * dcServiceHandle;
    inline std::shared_ptr<DcResponse> StringToDcResponse(MessageType t, std::string str){
        DcResponse msg = {};
        msg.set_type(t);
        msg.set_message_data(str.data(), str.size());
        msg.set_message_size(str.size());
        return std::make_shared<DcResponse>(msg);
    }
};