//
// Created by nkaffine on 12/10/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#include "Fulfil.Dispense/dispense/dispense_processing_queue_predicate.h"

using fulfil::dispense::DispenseProcessingQueuePredicate;
using fulfil::dispense::commands::DispenseResponse;
using fulfil::dispense::commands::DispenseRequest;

DispenseProcessingQueuePredicate::DispenseProcessingQueuePredicate(std::shared_ptr<std::string> command_id)
{
  this->command_id = command_id;
}

bool DispenseProcessingQueuePredicate::should_keep(std::shared_ptr<DispenseRequest> request)
{
  return *request->command_id != *this->command_id;
}
