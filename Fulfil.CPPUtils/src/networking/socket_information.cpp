//
// Created by nkaffine on 12/9/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#include "Fulfil.CPPUtils/networking/socket_information.h"
using fulfil::utils::networking::SocketInformation;

SocketInformation::SocketInformation(unsigned short port)
{
  this->port = port;
}