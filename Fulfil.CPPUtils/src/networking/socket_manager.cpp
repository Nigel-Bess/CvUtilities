//
// Created by nkaffine on 12/9/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#include "Fulfil.CPPUtils/networking/socket_manager.h"
#include "Fulfil.CPPUtils/logging.h"
#include "Fulfil.CPPUtils/comm/GrpcService.h"

#include <sys/socket.h>
#include <memory>
#include <iostream>
#include <unistd.h>

using fulfil::utils::Logger;
using fulfil::utils::networking::SocketManager;


//TODO throw errors and catch upstream instead
SocketManager::SocketManager(std::shared_ptr<fulfil::utils::networking::SocketInformation> socket_information)
{
  this->socket_information = socket_information;
  this->service_ = std::make_shared<GrpcService>();
}

/**
 * Disconnect from the socket
 */
void SocketManager::Disconnect()
{
  Logger::Instance()->Info("Disconnecting from socket:  {}", this->connection_fd);

}

void SocketManager::create_socket()
{
  this->service_->Run(socket_information->port);
}

void SocketManager::connect_socket()
{

}

/**
 * Destructor disconnects from the socket
 */
SocketManager::~SocketManager()
{
  Disconnect();
}
