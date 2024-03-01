//
// Created by nkaffine on 12/12/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
#include <Fulfil.CPPUtils/networking/socket_command_header.h>
#include <memory>
#include<gtest/gtest.h>

TEST(headerSerializationTests, testSendingHeader)
{
  fulfil::utils::networking::SocketCommandHeader header;
  const char* id = "001000000000";
  memcpy(&header.command_id, &id, 12);
  header.bytesleft = 32;
  int pipe_fd[2];
  char buf[14];
  pid_t cpid;
  pipe(pipe_fd);
  cpid = fork();
  if(cpid == 0)
  {
    //In child
    //Closing writen end
    close(pipe_fd[1]);
    read(pipe_fd[0], &buf, 14);
    for(int i = 0; i < 14; i++)
    {
      EXPECT_EQ(buf[i], ((char*)&header)[i]);
    }
    close(pipe_fd[0]);
    exit(0);
  }
  else
  {
    close(pipe_fd[0]);
    write(pipe_fd[1], &header, sizeof(fulfil::utils::networking::SocketCommandHeader));
    close(pipe_fd[1]);
    wait(NULL);
  }
}