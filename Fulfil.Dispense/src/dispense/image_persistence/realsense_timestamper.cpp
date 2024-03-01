//
// Created by nkaffine on 12/12/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#include "Fulfil.Dispense/dispense/image_persistence/realsense_timestamper.h"

using fulfil::dispense::imagepersistence::RealsenseTimestamper;

std::shared_ptr<std::string> RealsenseTimestamper::get_timestamp()
{
  time_t rawtime;
  struct tm * timeinfo;
  char buffer[80];

  time (&rawtime);
  timeinfo = localtime(&rawtime);

  strftime(buffer,sizeof(buffer),"%d-%m-%Y %H:%M:%S",timeinfo);
  std::shared_ptr<std::string> string = std::make_shared<std::string>(buffer);
  return string;
}