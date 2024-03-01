#pragma once

#include <iostream>
#include <memory>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include "GrpcService.h"

#include "depthCams.pb.h"
#include "depthCams.grpc.pb.h"

using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;

using namespace DepthCameras;

class SbcClient final{
    public:
        SbcClient(std::shared_ptr<Channel> channel): stub_(DcApiService::NewStub(channel)) {}

        std::shared_ptr<grpc::ClientAsyncReaderWriter<DcRequest, DcResponse>> DepthCamStream(ClientContext * ctx, CompletionQueue * cq) {

            std::shared_ptr<grpc::ClientAsyncReaderWriter<DcRequest, DcResponse>> 
                        stream(stub_->AsyncDcApi(ctx, cq, this));

            return stream;
        }
        ClientContext context_;

    private:
        std::unique_ptr<DcApiService::Stub> stub_;
        bool _cancel_stream = false;
};

class ClientStream final{
    public:
        Status status_;
        ClientStream(std::string ip, uint16_t port){
            std::string endpoint= absl::StrFormat("%s:%d", ip, port);
            SbcClient client(grpc::CreateChannel(endpoint, grpc::InsecureChannelCredentials()));
            stream_ = client.DepthCamStream(&ctx, &cq);
            ReadReponse();
        };
        void Write(std::string cmd_id, std::string message){
            std::cout << "Sending message: " << message << std::endl;
            request_.set_type(MESSAGE_TYPE_GENERIC_QUERY);
            request_.set_command_id(cmd_id);
            request_.set_message_data(message.data(), message.length());
            request_.set_message_size(message.length());
            stream_->Write(request_, tag);
            ProcessQueue();
        }
        std::string ReadReponse(){
            ProcessQueue();
            if(!ok)return "";
            stream_->Read(&response_, tag);
            if(response_.type() == MESSAGE_TYPE_UNSPECIFIED){
                std::cout << "empty response, reading again" << std::endl;
                ReadReponse();
            }
            else{
                std::string str(response_.message_data().data(), response_.message_size());
                std::cout << "SbcClient response [" << response_.command_id()  << "]: " << MessageType_Name(response_.type()) << " | " << str << std::endl;
                return str;
            }
            return "empty";
        }

        ClientContext ctx;
        std::shared_ptr<grpc::ClientAsyncReaderWriter<DcRequest, DcResponse>> stream_;
        CompletionQueue cq;
        ~ClientStream(){  }
        
    private:
        void ProcessQueue(){
            cq.Next(&got_tag, &ok);
        }
        void* got_tag;
        void* tag;
        bool ok = false;
        DcRequest request_;
        DcResponse response_;
};

