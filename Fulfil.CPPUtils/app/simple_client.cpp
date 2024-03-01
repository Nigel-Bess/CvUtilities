//
// Created by nkaffine on 12/9/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#define PORT 9500
#include <Fulfil.CPPUtils/networking/socket_command_header.h>
#include <cstring>
#include <iostream>

int main(int argc, char const *argv[])
{
  fulfil::utils::networking::SocketCommandHeader header;
  memcpy(&header.command_id, "012345678901", 12);
  header.bytesleft = sizeof(int);

  int sock = 0, valread;
  struct sockaddr_in serv_addr;
  char buffer[1024] = {0};
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("\n Socket creation error \n");
    return -1;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  // Convert IPv4 and IPv6 addresses from text to binary form
  if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
  {
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    printf("\nConnection Failed \n");
    return -1;
  }
  char* sending_buffer = new char(sizeof(fulfil::utils::networking::SocketCommandHeader) + sizeof(int));
  std::memcpy(sending_buffer, &header, sizeof(fulfil::utils::networking::SocketCommandHeader));
  int request = 8;
  std::memcpy(&sending_buffer[sizeof(fulfil::utils::networking::SocketCommandHeader)], &request, sizeof(int));

  send(sock , sending_buffer, sizeof(fulfil::utils::networking::SocketCommandHeader) + sizeof(int), 0 );
  printf("Header was sent\n");
  while(true)
  {
    valread = read( sock , buffer, 1024);
    if(valread != -1)
    {
      fulfil::utils::networking::SocketCommandHeader header2{};
      std::memcpy(&header2, buffer, sizeof(fulfil::utils::networking::SocketCommandHeader));
      std::cout << header2.command_id << std::endl;
      printf("%s\n", &buffer[sizeof(fulfil::utils::networking::SocketCommandHeader)]);
      break;
    }
  }
  return 0;
}

