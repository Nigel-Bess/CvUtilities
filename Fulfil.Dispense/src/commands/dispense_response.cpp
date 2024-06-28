//
// Created by nkaffine on 12/11/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved

#include "Fulfil.Dispense/commands/dispense_response.h"
#include <Fulfil.CPPUtils/networking.h>
#include <Fulfil.CPPUtils/logging.h>

using fulfil::utils::networking::SocketCommandHeader;
using fulfil::dispense::commands::DispenseResponse;

int DispenseResponse::header_size()
{
  return sizeof(SocketCommandHeader);
}

void * DispenseResponse::header()
{
  SocketCommandHeader* header = new SocketCommandHeader();
  memset(header, 0, sizeof(SocketCommandHeader));
  header->bytesleft = this->dispense_payload_size();
  memcpy(header->command_id, this->get_command_id()->c_str(), 24);
  return header;
}

int DispenseResponse::payload_size()
{
  return this->dispense_payload_size();
}

void * DispenseResponse::payload()
{
  void* payload = malloc(this->dispense_payload_size());
  memset(payload, 0, this->dispense_payload_size());
  std::memcpy(payload, this->dispense_payload()->c_str(), this->dispense_payload_size());
  return payload;
}
