//
// Created by nkaffine on 12/10/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DISPENSE_DISPENSE_PROCESSING_QUEUE_PREDICATE_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DISPENSE_DISPENSE_PROCESSING_QUEUE_PREDICATE_H_
#include <memory>
#include <Fulfil.CPPUtils/processing_queue/processing_queue_predicate.h>
#include <Fulfil.Dispense/commands/dispense_request.h>
#include <Fulfil.Dispense/commands/dispense_response.h>

namespace fulfil
{
namespace dispense {
/**
 * The purpose of this class is to be used in the processing
 * queue used in the networking section of the code to filter
 * out commands that match the command id.
 */
class DispenseProcessingQueuePredicate :
    public fulfil::utils::processingqueue::ProcessingQueuePredicate<std::shared_ptr<fulfil::dispense::commands::DispenseRequest>>
{
 private:
  /**
   * The command id that will be used to compare with
   * commands ids, if they match, it will return that
   * they should not be kept.
   */
  std::shared_ptr<std::string> command_id;
 public:
  /**
   * DispenseProcessingQueuePredicate constructor.
   * @param command_id pointer to string with command id that
   * will be used to filter out commands with the same id.
   */
  explicit DispenseProcessingQueuePredicate(std::shared_ptr<std::string> command_id);

  bool should_keep(std::shared_ptr<fulfil::dispense::commands::DispenseRequest> request) override;
};
} // namespace dispense
} // namespace fulfil

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DISPENSE_DISPENSE_PROCESSING_QUEUE_PREDICATE_H_
