//
// Created by nkaffine on 12/9/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_NETWORKING_SOCKET_NETWORK_MANAGER_H_
#define FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_NETWORKING_SOCKET_NETWORK_MANAGER_H_

#include <memory>
#include "socket_manager.h"
#include "socket_network_manager_delegate.h"
#include "socket_response.h"
#include "socket_command_parser.h"
#include <thread>
#include <iostream>
#include <mutex>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include "socket_command_header.h"
#include "socket_errors.h"
#include "socket_command_parsing_errors.h"
#include "Fulfil.CPPUtils/logging.h"
#include <FulfilMongoCpp/mongo_objects/mongo_object_id.h>


namespace fulfil
{
namespace utils
{
namespace networking
{
/**
 * The purpose of this class is to provide a generic implementation of
 * reading and writing to sockets in a standard format (headers with
 * payloads).
 * @tparam Request the type that represents a request.
 * @note the implementation of this class is in the header for flexbility
 * in what can be passed as the generic argument in the class.
 */
template <class Request>
class SocketNetworkManager
{
 private:
  /**
   * Boolean that determines where the manager should continue listening on the socket.
   */
  bool should_listen = true;
  /**
   * Mutex to make sure the should listen variable is only accessed by one thread at a time.
   */
  std::mutex should_listen_mutex;
  /**
   * Mutex to make sure only one thread is trying to access the delegate.
   */
  std::mutex delegate_mutex;
  /**
   * The object that handles the creation and connecting to sockets.
   */
  std::shared_ptr<fulfil::utils::networking::SocketManager> socket_manager;
  /**
   * The object that parses the commands from the socket headers.
   */
  std::shared_ptr<fulfil::utils::networking::SocketCommandParser<Request>> command_parser;
  /**
   * Handles the loop to listen to the socket and process the requests that are read.
   * This is called by the start listening function and will be stopped by calling
   * the stop listening function.
   */
  void listen();
  /**
   * Attempts to read the header from the socket.
   * @return pointer to the header that was read from the socket.
   * @throws SocketDisconnectionException if the socket is disconnected.
   * @throws SocketHeaderParsingException if the bytes read from the socket was greater or less
   * than the size of the header.
   */
  std::shared_ptr<fulfil::utils::networking::SocketCommandHeader> read_header();
  /**
   * Attempts to process that command outlined by the header.
   * @param header pointer to the header of the socket command.
   * @throws InvalidSocketPayloadSizeException if the header has invalid bytes left parameter.
   * @throws UnexpectedSocketReadSizeException if the bytes read from the socket
   * don't match the bytesleft parameter of the header.
   * @throws InvalidCommandFormatException when the command parser fails to parse.
   */
  void process_command(std::shared_ptr<fulfil::utils::networking::SocketCommandHeader> header);
  /**
   * Attempts to process the payload.
   * @param payload pointer to string containing the command payload.
   * @param command_id pointer to string with command id.
   * @throws InvalidCommandFormatException when the command parser fails to parse.
   */
  void process_payload(std::shared_ptr<std::string> payload, std::shared_ptr<std::string> command_id);
  /**
   * Handles the SocketHeaderParsingException by calling did_receive_invalid_request on
   * the delegate if there is one and printing the error to stdout
   * @param error const reference to the error
   */
  void handle_error(const fulfil::utils::networking::errors::SocketHeaderParsingException& error);
  /**
   * Handles the SocketDisconnectionException by printing the error to stdout
   * @param error const reference to the error
   */
  void handle_error(const fulfil::utils::networking::errors::SocketDisconnectionException& error);
  /**
   * Handles UnexpectedSocketReadSizeException by printing the error to stdout and
   * calling did_receive_invalid_request on the delegate if there is one.
   * @param error const reference to the error
   */
  void handle_error(const fulfil::utils::networking::errors::UnexpectedSocketReadSizeException& error);
  /**
   * Handles the InvalidSocketPayloadSizeException by printing the error to stdout and
   * calling did_receive_invalid_request on the delegate if there is one.
   * @param error const reference to the error
   */
  void handle_error(const fulfil::utils::networking::errors::InvalidSocketPayloadSizeException& error);
  /**
   * Handles the InvalidCommandFormatException by printing the error to stdout and
   * calling did_receive_invalid_request on the delegate if there is one.
   * @param error const reference to the error
   */
  void handle_error(const fulfil::utils::networking::errors::InvalidCommandFormatException& error);
 public:
  /**
   * Delegate that handles the logic for when a request is received.
   */
  std::weak_ptr<fulfil::utils::networking::SocketNetworkManagerDelegate<Request>> delegate;
  /**
   * SocketNetworkManager Constructor
   * @param socket_manager the object that will handle the creation and connecting to sockets.
   * @param command_parser the object that will parse headers into commands to be passed to the delegate.
   */
  SocketNetworkManager(std::shared_ptr<fulfil::utils::networking::SocketManager> socket_manager,
      std::shared_ptr<fulfil::utils::networking::SocketCommandParser<Request>> command_parser);
  /**
   * Initiates the listening process of the socket, this function will create a new thread
   * for listening to the socket so it will not block the current thread..
   */
  void start_listening();
  /**
   * Sends a response via the socket.
   * @param response the response that will be sent.
   */
  void send_response(std::shared_ptr<fulfil::utils::networking::SocketResponse> response);
  /**
   * Causes the thread(s) that are listening to the socket to stop listening to the socket.
   */
  void stop_listening();
};

template <class Request>
void SocketNetworkManager<Request>::listen()
{
  //Creating and start gRPC server
  this->socket_manager->create_socket();
  // Initializing should listen to true in case stop listening was called previously.
  this->should_listen_mutex.lock();
  this->should_listen = true;
  this->should_listen_mutex.unlock();
  //Initializing local should listen variable that will be updated to consider multi threaded code.
  bool should_listen = true;
  while(should_listen)
  {
    fulfil::utils::Logger::Instance()->Debug("listening");
    while(true){
        while(this->socket_manager->service_.HasNewRequest()){
            auto request = this->socket_manager->service_.GetNextRequest();
            fulfil::utils::Logger::Instance()->Info("New request {}: {}", request->command_id(), MessageType_Name(request->type()));
            this->process_payload(std::make_shared<std::string>(request->message_data().data(), request->message_size()), std::make_shared<std::string>(request->command_id()));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    //Updates the should listen variable based on the should listen variable on the object.
    this->should_listen_mutex.lock();
    should_listen = this->should_listen;
    this->should_listen_mutex.unlock();
  }
}


template <class Request>
void SocketNetworkManager<Request>::process_payload(std::shared_ptr<std::string> payload, std::shared_ptr<std::string> command_id)
{
  try
  { 
    auto oid = bsoncxx::oid(command_id.get()->c_str()).to_string();
    // Request request = this->command_parser->parse_payload(payload, command_id);
    Logger::Instance()->Debug("Request object created, Payload id {}",oid);
    this->delegate_mutex.lock();
    if (!this->delegate.expired())
    {
      //Delegate is not null so use it to process the request
      auto tmp = this->delegate.lock();
      Logger::Instance()->Debug("Calling handle_request in SocketNetworkManager {} ", oid);
      tmp->handle_request(payload, command_id);
      Logger::Instance()->Debug("Call DONE handle_request in SocketNetworkManager {} ", oid);
    }
    else Logger::Instance()->Error("Delegate expired, can't process request! did_receive_request in SocketNetworkManager");
    this->delegate_mutex.unlock();
  }
  catch (const fulfil::utils::networking::errors::InvalidCommandFormatException& command_format_exception)
  {
    this->handle_error(command_format_exception);
  }
}

template <class Request>
SocketNetworkManager<Request>::SocketNetworkManager(std::shared_ptr<fulfil::utils::networking::SocketManager> socket_manager,
                                                    std::shared_ptr<fulfil::utils::networking::SocketCommandParser<Request>> command_parser)
{
  this->socket_manager = socket_manager;
  this->command_parser = command_parser;
}

template <class Request>
void SocketNetworkManager<Request>::start_listening()
{
  this->should_listen_mutex.lock();
  this->should_listen = true;
  this->should_listen_mutex.unlock();
  std::thread(&SocketNetworkManager::listen, this).detach();
}

template <class Request>
void SocketNetworkManager<Request>::send_response(std::shared_ptr<fulfil::utils::networking::SocketResponse> response)
{
    auto api_resp = std::make_shared<DcResponse>();
    api_resp->set_command_id((char *)response->header(), 24);//TODO make response header a string
    fulfil::utils::Logger::Instance()->Debug("Response id: {}", api_resp->command_id());
    api_resp->set_type(response->message_type);
    api_resp->set_message_size(response->payload_size());
    api_resp->set_message_data(response->payload(), response->payload_size());
    this->socket_manager->service_.QueueResponse(api_resp);

}

template <class Request>
void SocketNetworkManager<Request>::stop_listening()
{
  this->should_listen_mutex.lock();
  this->should_listen = false;
  this->should_listen_mutex.unlock();
}

template<class Request>
void SocketNetworkManager<Request>::handle_error(const fulfil::utils::networking::errors::SocketHeaderParsingException &error)
{
  fulfil::utils::Logger::Instance()->Info("SocketNetwork manager -> handle error 2: {}", error.what());
  this->delegate_mutex.lock();
  if(!this->delegate.expired())
  {
    auto tmp_delegate = this->delegate.lock();
    tmp_delegate->did_receive_invalid_request();
  }
  this->delegate_mutex.unlock();
}

template<class Request>
void SocketNetworkManager<Request>::handle_error(const fulfil::utils::networking::errors::SocketDisconnectionException &error)
{
  fulfil::utils::Logger::Instance()->Info("SocketNetwork manager -> handle error 3: {}", error.what());
  this->socket_manager->Disconnect();
  //stop_listening();
}

template<class Request>
void SocketNetworkManager<Request>::handle_error(const fulfil::utils::networking::errors::UnexpectedSocketReadSizeException &error)
{
  fulfil::utils::Logger::Instance()->Info("SocketNetwork manager -> handle error 4: {}", error.what());
  this->delegate_mutex.lock();
  if(!this->delegate.expired())
  {
    auto tmp_delegate = this->delegate.lock();
    tmp_delegate->did_receive_invalid_request(error.command_id);
  }
  this->delegate_mutex.unlock();
}

template<class Request>
void SocketNetworkManager<Request>::handle_error(const fulfil::utils::networking::errors::InvalidSocketPayloadSizeException &error)
{
  fulfil::utils::Logger::Instance()->Info("SocketNetwork manager -> handle error 5: {}", error.what());
  this->delegate_mutex.lock();
  if(!this->delegate.expired())
  {
    auto tmp_delegate = this->delegate.lock();
    tmp_delegate->did_receive_invalid_request(error.command_id);
  }
  this->delegate_mutex.unlock();
}

template<class Request>
void SocketNetworkManager<Request>::handle_error(const fulfil::utils::networking::errors::InvalidCommandFormatException &error)
{
  fulfil::utils::Logger::Instance()->Info("SocketNetwork manager -> handle error 6: {}", error.what());
  this->delegate_mutex.lock();
  if(!this->delegate.expired())
  {
    auto tmp_delegate = this->delegate.lock();
    tmp_delegate->did_receive_invalid_request(error.command_id);
  }
  this->delegate_mutex.unlock();
}

} // namespace networking
} // namespace utils
} // namespace fulfil

#endif //FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_NETWORKING_SOCKET_NETWORK_MANAGER_H_
