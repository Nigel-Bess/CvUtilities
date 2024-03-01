//
// Created by nkaffine on 12/9/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_PROCESSING_QUEUE_PROCESSING_QUEUE_DELEGATE_H_
#define FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_PROCESSING_QUEUE_PROCESSING_QUEUE_DELEGATE_H_

namespace fulfil
{
namespace utils
{
namespace processingqueue
{
/**
 * The purpose of this class is to allow separation between handling a multi threaded
 * processing queue from the logic of actually processing elements in the queue. This
 * delegate allows for custom implementation of processing a request in a queue and
 * processing the response.
 * @tparam Request the type that represents requests in the queue
 * @tparam Response the type that represents responses in the queue.
 */
template <typename Request, typename Response>
class ProcessingQueueDelegate
{
 public:
  /**
   * Given the response, the implementation of this function
   * should perform any required actions with the response.
   * @param response the response object that was the result
   * of processing a request.
   */
  virtual void send_response(Response response) = 0;
  /**
   * Given a request, the implementation of this function
   * should perform any required operations on the request
   * to generate a response that will be handled by
   * the delegate.
   * @param request the request that needs to be processed.
   * @return the response from processing the request.
   */
  virtual Response process_request(Request request) = 0;
};
}
}
}

#endif //FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_PROCESSING_QUEUE_PROCESSING_QUEUE_DELEGATE_H_
