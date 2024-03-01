#include "Fulfil.CPPUtils/comm/GrpcService.h"



std::string GetShortId(const std::string id){
    auto length = id.length();
    if(length <= 6)return id;
    std::string newId = id.substr(length - 6, length - 1);
    return newId;
}

void DepthCamServiceImpl::Process(){
    auto writes = tasks_->WritesLeft();
    switch(status_){
        case CREATE:
            status_ = WRITE;
            aservice_->RequestDcApi(&ctx_, &stream_, cq_, cq_, this);
            QueueHelloMessage();
            conn_time_ = CurrentTime();
            Connected = false;
        break;
        case READ:{
            stream_.Read(&request_, this);
            status_ = RESPONSE;  
            conn_time_ = CurrentTime();
        }
        break;
        case RESPONSE:
            ///gRPC async handles one request at a time, if client / server are both waiting on a read, lockup happens
            tasks_->PackRead(request_);//queue read, put ack in front
        case WRITE:{
            writes = tasks_->WritesLeft();
            if(writes > 0){
                stream_.Write(tasks_->GetNextWrite(), this);
                writes--;
            }
            CheckConnection();
            conn_time_ = CurrentTime();
            status_ = writes > 0 ? WRITE : READ;
        }
        break;
        case FINISH:
            tasks_->Clear();
            Logger::Instance()->Info("gRPC client disconnected!");
            delete this;
        break;
    }
}

void DepthCamServiceImpl::QueueHelloMessage(){
    auto hb = std::make_shared<DcResponse>();
    hb->set_type(MESSAGE_TYPE_HEARTBEAT);
    hb->set_command_id(GetTxObjectIdString()); 
    tasks_->PackWrite(hb);
}

void DepthCamServiceImpl::SendMessage(std::shared_ptr<DcResponse> msg){
    if(!Connected)return;
    //msg->set_id(GetTxObjectIdString());
    tasks_->PackWrite(msg);
}

void GrpcService::Run(uint16_t port){
    std::string server_address = absl::StrFormat("0.0.0.0:%d", port);
    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    
    builder.RegisterService(&async_service_);
    cq_ = builder.AddCompletionQueue();
    server_ = builder.BuildAndStart();
    Logger::Instance()->Info("GrpcServer listening on {}", server_address);
   
    dcServiceHandle = new DepthCamServiceImpl(&async_service_, cq_.get(), &tasks_);
    
    std::thread wthread(&GrpcService::HandleNewMessages, this);
    wthread.detach();
    std::thread rpcThread(&GrpcService::HandleRawRpcs, this);
    rpcThread.detach();
    // HandleRawRpcs();//block
    
}

void GrpcService::HandleNewMessages(){
    sem_init(&_cmd_wait, 0, 1);
    while(true){
        sem_wait(&_cmd_wait);     
        //check for any status messages first
        while(tasks_.HasNewStatus()){
            auto msg = StringToDcResponse(MESSAGE_TYPE_COMMAND_STATUS, tasks_.GetStatusMessage());
            dcServiceHandle->SendMessage(msg);
        }

        // while(tasks_.HasReads()){
        //     auto msg = tasks_.GetNextRequest();
        //     Logger::Instance()->Info("New gRPC Command [{}]--> {} | {}", GetShortId(msg->id()), 
        //                 MessageType_Name(msg->type()), DcCommandType_Name(msg->dc_cmd_type()));
        // }
        std::this_thread::sleep_for(std::chrono::microseconds(50));
        
    }
}


void GrpcService::HandleRawRpcs(){
    void *tag; // uniquely identifies a request.
    bool ok; 
    while (true){
        GPR_ASSERT(cq_->Next(&tag, &ok));
        auto client = static_cast<AsyncConnection *>(tag);
        if(!ok)
            client->status_ = FINISH;
        
        client->Process();

        if(!ok)
            dcServiceHandle = new DepthCamServiceImpl(&async_service_, cq_.get(), &tasks_);
        sem_post(&_cmd_wait);
        std::this_thread::sleep_for(std::chrono::microseconds(50));
    }
}
