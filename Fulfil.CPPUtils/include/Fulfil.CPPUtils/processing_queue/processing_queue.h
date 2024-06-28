#ifndef FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_PROCESSING_QUEUE_PROCESSING_QUEUE_H_
#define FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_PROCESSING_QUEUE_PROCESSING_QUEUE_H_
#include <Fulfil.CPPUtils/processing_queue/processing_queue_delegate.h>
#include <Fulfil.CPPUtils/processing_queue/processing_queue_predicate.h>
#include <memory>
#include <queue>
#include <mutex>
#include <thread>
#include <stack>
#include <Fulfil.CPPUtils/logging.h>

namespace fulfil::utils::processingqueue
{
    /**
     * The purpose of this template is to provide an abstract implementation
     * of a process queue that processes requests and returns responses which
     * are generic types handled by the delegate to process.
     * @tparam Request the type that will represent requests.
     * @tparam Response the type that will represent responses.
     * @note The implementation of this class is in the header as a result of
     * how c++ compiles generic classes. The other option to prevent linking
     * issues was to explicitly enumerate all types that will be used as
     * Request and Response which is very inflexible.
     */
    template<typename Request, typename Response>
    class ProcessingQueue
    {
        private:
            /**
            * The inner processing queue that has pairs of requests and ints where the int
            * is the number of times the request should be repeatedly processed after the
            * current processing.
            */
            std::unique_ptr<std::queue<std::shared_ptr<Request>>> request_queue;
            /**
            * A mutex to handle the multi threaded access to the queue.
            */
            std::mutex request_queue_mutex;
            /**
            * A mutex to handle the multi threaded access to the flag telling the class
            * to process the processing queue.
            */
            std::mutex processing_mutex;
            /**
            * A mutex to handle to multi threaded access to the delegate.
            */
            std::mutex delegate_mutex;
            /**
            * Boolean flag to determine whether the queue should be processing requests.
            */
            bool should_process = true;
        public:
            /**
            * A weak pointer to the delegate which will be called while processing the queue.
            * @note this is a weak pointer to prevent retain cycles since there are sitatutions
            * where it makes sense for the delegate to have a reference to the queue.
            */
            std::weak_ptr<fulfil::utils::processingqueue::ProcessingQueueDelegate<Request, Response>> delegate;
            /**
            * Constructor that initializes the inner queue for processing requests.
            */
            ProcessingQueue();
            /**
            * Pushes a request on to the queue with the provided retry limit.
            * @param request the generic request that will be handed off to the delegate
            * to process when it comes up in the queue.
            */
            void push(Request request);
            /**
            * Initializes the while loop for processing the queue, this is a synchronous function so
            * the program will hang on the function call unless you spawn a thread to call it within
            * the thread.
            * @param millisecond_interval millisecond delay between each iteration of the while
            * loop.
            * @note the inner loop will only stop when the program ends or the stop_processing method
            * is called.
            */
            void start_processing(int millisecond_interval);
            /**
            * Tell the queue to stop processing. This will only have an effect if the
            * queue is already in the middle of processing.
            */
            void stop_processing();
            /**
            * This function throws away any of the requests in the queue that should
            * not be kept according to the provided predicate.
            * @param predicate object that provides a function to determine whether or not
            * a request should be kept in the processing queue.
            */
            void purge_queue(std::shared_ptr<fulfil::utils::processingqueue::ProcessingQueuePredicate<Request>> predicate);
    };

    template <typename Request, typename Response>
    ProcessingQueue<Request, Response>::ProcessingQueue()
    {
        //Initializing an empty queue.
        this->request_queue = std::make_unique<std::queue<std::shared_ptr<Request>>>();
    }

    template<typename Request, typename Response>
    void ProcessingQueue<Request, Response>::push(Request request)
    {
        fulfil::utils::Logger::Instance()->Trace("Processing Queue push method called");
        std::lock_guard<std::mutex>(this->request_queue_mutex);
        this->request_queue->push(std::make_shared<Request>(request));
        fulfil::utils::Logger::Instance()->Debug("Request was pushed to processing queue {}", *request->request_id);
    }

    template<typename Request, typename Response>
    void ProcessingQueue<Request, Response>::start_processing(int millisecond_interval)
    {
        while(true)
        {
            this->request_queue_mutex.lock();
            if(!this->request_queue->empty())
            {
                fulfil::utils::Logger::Instance()->Trace("Processing queue recognizes it is no longer empty");
                Request request =  *this->request_queue->front();
                this->request_queue->pop();
                this->request_queue_mutex.unlock();
                //Process the request
                // this->delegate_mutex.lock();
                if(!this->delegate.expired())
                {
                    std::shared_ptr<ProcessingQueueDelegate<Request, Response>> tmp = this->delegate.lock();
                    fulfil::utils::Logger::Instance()->Debug("Processing queue calls process_request in dispense_manager {}", *request->request_id);
                    Response response = tmp->process_request(request);
                    fulfil::utils::Logger::Instance()->Debug("Processing queue calls send_response {}", *request->request_id);
                    tmp->send_response(response);
                    // this->delegate_mutex.unlock();
                }
                // else { this->delegate_mutex.unlock(); }
            }
            else
            {
                this->request_queue_mutex.unlock();
            }
            if(millisecond_interval > 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(millisecond_interval));
            }
        }
    }

    template<typename Request, typename Response>
    void ProcessingQueue<Request, Response>::stop_processing()
    {
        std::lock_guard<std::mutex> lock(this->processing_mutex);
        this->should_process = false;
    }

    template<typename Request, typename Response>
    void ProcessingQueue<Request,Response>::purge_queue(std::shared_ptr<ProcessingQueuePredicate<Request>> predicate)
    {
        std::lock_guard<std::mutex> lock(this->request_queue_mutex);
        //Using stack to preserve order of the queue O(N)
        std::unique_ptr<std::stack<std::shared_ptr<Request>>> tmp_stack =
          std::make_unique<std::stack<std::shared_ptr<Request>>>();
        while(!this->request_queue->empty())
        {
            std::shared_ptr<Request> item = this->request_queue->front();
            if(predicate->should_keep(*item))
            {
              tmp_stack->push(item);
            }
            this->request_queue->pop();
        }
        while(!tmp_stack->empty())
        {
            this->request_queue->push(tmp_stack->top());
            tmp_stack->pop();
        }
    }

} // namespace fulfil::utils::processingqueue

#endif //FULFIL_UTILS_INCLUDE_FULFIL_CPPUTILS_PROCESSING_QUEUE_H_
