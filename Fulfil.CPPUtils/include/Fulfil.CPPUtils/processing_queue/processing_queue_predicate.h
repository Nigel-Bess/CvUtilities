//
// Created by nkaffine on 12/9/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_PROCESSING_QUEUE_PROCESSING_QUEUE_PREDICATE_H_
#define FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_PROCESSING_QUEUE_PROCESSING_QUEUE_PREDICATE_H_

namespace fulfil
{
namespace utils
{
namespace processingqueue
{
/**
 * The purpose of this class is to outline functionality for classes
 * to filter out and keep specific requests in a processing queue;
 * @tparam Request the type that represents requests.
 */
template <class Request> class ProcessingQueuePredicate
{
 public:
  /**
   * Returns whether the given request should be included
   * in the queue or should be removed from the queue.
   * @param request
   * @return true if the request should be included in the queue,
   * false if it should be filtered out.
   */
  virtual bool should_keep(Request request)=0;
};
} // namespace processingqueue
} // namespace utils
} // namespace fulfil

#endif //FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_PROCESSING_QUEUE_PROCESSING_QUEUE_PREDICATE_H_
