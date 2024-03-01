//
// Created by nkaffine on 12/9/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
#include <memory>
#include <gtest/gtest.h>
#include <Fulfil.CPPUtils/processing_queue.h>
#include <stack>
#include <thread>

using fulfil::utils::processingqueue::ProcessingQueuePredicate;
using fulfil::utils::processingqueue::ProcessingQueueDelegate;
using fulfil::utils::processingqueue::ProcessingQueue;
using std::make_shared;

class SpyProcessingQueuePredicate : public ProcessingQueuePredicate<int>
{
 private:
  bool (*predicate)(int);
 public:
  explicit SpyProcessingQueuePredicate(bool (*predicate)(int))
  {
    this->predicate = predicate;
  }

  bool should_keep(int i) override
  {
    return this->predicate(i);
  }
};

class SpyCharProcessingQueuePredicate : public ProcessingQueuePredicate<std::shared_ptr<char*>>
{
 private:
  bool (*predicate)(std::shared_ptr<char*>);
 public:
  explicit SpyCharProcessingQueuePredicate(bool (*predicate)(std::shared_ptr<char*>))
  {
    this->predicate = predicate;
  }

  bool should_keep(std::shared_ptr<char*> word) override
  {
    return this->predicate(word);
  }
};

 class SpyProcessingQueueDelegate : public ProcessingQueueDelegate<int, int>, public std::enable_shared_from_this<SpyProcessingQueueDelegate>
{
 public:
  std::unique_ptr<std::stack<int>> responses;
  SpyProcessingQueueDelegate()
  {
    this->responses = std::make_unique<std::stack<int>>();
  }
  void send_response(int response)
  {
    this->responses->push(response);
  }
  int process_request(int request)
  {
    return request;
  }
};

TEST(processingQueueTests, testSimpleProcessing)
{
  std::shared_ptr<SpyProcessingQueueDelegate> delegate = std::make_shared<SpyProcessingQueueDelegate>();
  std::shared_ptr<ProcessingQueue<int, int>> queue = std::make_shared<ProcessingQueue<int, int>>();
  queue->delegate = delegate->weak_from_this();
  queue->push(1, 0);
  queue->push(2, 0);
  queue->push(3, 0);
  std::thread t([queue](){
    queue->start_processing(0);
  });
  t.detach();
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  queue->stop_processing();
  EXPECT_EQ(delegate->responses->size(), 3);
}

TEST(processingQueueTests, testPurgingQueue)
{
  std::shared_ptr<SpyProcessingQueueDelegate> delegate = std::make_shared<SpyProcessingQueueDelegate>();
  std::shared_ptr<ProcessingQueue<int, int>> queue = std::make_shared<ProcessingQueue<int, int>>();
  queue->delegate = delegate->weak_from_this();
  queue->push(1, 0);
  queue->push(2, 0);
  queue->push(1, 0);

  std::shared_ptr<SpyProcessingQueuePredicate> predicate = std::make_shared<SpyProcessingQueuePredicate>([](int i){return i != 1;});
  //Purge
  queue->purge_queue(predicate);
  std::thread t([queue](){
    queue->start_processing(0);
  });
  t.detach();
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  queue->stop_processing();
  EXPECT_EQ(delegate->responses->size(), 1);
}

typedef std::shared_ptr<char*> SharedString;
class CharSpyProcessingQueueDelegate : public ProcessingQueueDelegate<SharedString, SharedString>,
    public std::enable_shared_from_this<CharSpyProcessingQueueDelegate>
{
 public:
  std::unique_ptr<std::stack<SharedString>> responses;
  CharSpyProcessingQueueDelegate()
  {
    this->responses = std::make_unique<std::stack<SharedString>>();
  }
  void send_response(SharedString response)
  {
    this->responses->push(response);
  }
  SharedString process_request(SharedString request)
  {
    return request;
  }
};

std::shared_ptr<char*> const_to_shared(const char* word)
{
  auto string = make_shared<char*>(new char[strlen(word) + 1]);
  memcpy(*string, word, strlen(word));
  memset(&(*string)[strlen(word)],0,1);
  return string;
}

TEST(processingQueueTests, testPurgingStringComparison)
{
  using namespace std;
  shared_ptr<CharSpyProcessingQueueDelegate> delegate = make_shared<CharSpyProcessingQueueDelegate>();
  shared_ptr<ProcessingQueue<SharedString, SharedString>> queue =
                                                              make_shared<ProcessingQueue<SharedString, SharedString>>();
  queue->delegate = delegate->weak_from_this();
  queue->push(const_to_shared("hello"), 0);
  queue->push(const_to_shared("goodbye"), 0);
  queue->push(const_to_shared("hello"), 0);
  queue->push(const_to_shared("goofy"), 0);
  queue->push(const_to_shared("heok"), 0);
  bool(*func)(SharedString) = [](SharedString s){
    int i = strcmp(*s, *const_to_shared("hello"));
    return i != 0;
  };
  std::shared_ptr<SpyCharProcessingQueuePredicate> predicate = std::make_shared<SpyCharProcessingQueuePredicate>(func);
  queue->purge_queue(predicate);
  std::thread t([queue](){
    queue->start_processing(0);
  });
  t.detach();
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  queue->stop_processing();
  EXPECT_EQ(delegate->responses->size(), 3);
}

TEST(processingQueueTests, testAddingToQueueWhilProcessing)
{
  std::shared_ptr<SpyProcessingQueueDelegate> delegate = std::make_shared<SpyProcessingQueueDelegate>();
  std::shared_ptr<ProcessingQueue<int, int>> queue = std::make_shared<ProcessingQueue<int, int>>();
  queue->delegate = delegate->weak_from_this();
  queue->push(1, 0);
  queue->push(2, 0);
  queue->push(1, 0);
  std::thread t([queue](){
    queue->start_processing(100);
  });
  t.detach();
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  queue->push(4, 0);
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  queue->stop_processing();
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  EXPECT_EQ(delegate->responses->size(), 4);

  EXPECT_EQ(delegate->responses->top(), 4);
  delegate->responses->pop();

  EXPECT_EQ(delegate->responses->top(), 1);
  delegate->responses->pop();

  EXPECT_EQ(delegate->responses->top(), 2);
  delegate->responses->pop();

  EXPECT_EQ(delegate->responses->top(), 1);
}

TEST(processingQueueTests, testPurgingQueueWhileProcessing)
{
  std::shared_ptr<SpyProcessingQueueDelegate> delegate = std::make_shared<SpyProcessingQueueDelegate>();
  std::shared_ptr<ProcessingQueue<int, int>> queue = std::make_shared<ProcessingQueue<int, int>>();
  queue->delegate = delegate->weak_from_this();
  for(int i = 0; i < 10; i++)
  {
    queue->push(1, 0);
  }
  std::thread t([queue](){
    queue->start_processing(10);
  });
  t.detach();
  std::shared_ptr<SpyProcessingQueuePredicate> predicate = std::make_shared<SpyProcessingQueuePredicate>([](int i){return i == 1;});
  queue->purge_queue(predicate);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  queue->stop_processing();
  EXPECT_LE(delegate->responses->size(), 10);
}

TEST(processingQueueTests, testRetyLimit)
{
  std::shared_ptr<SpyProcessingQueueDelegate> delegate = std::make_shared<SpyProcessingQueueDelegate>();
  std::shared_ptr<ProcessingQueue<int, int>> queue = std::make_shared<ProcessingQueue<int, int>>();
  queue->delegate = delegate->weak_from_this();
  for(int i = 0; i < 10; i++)
  {
    queue->push(i, i);
  }
  std::thread t([queue](){
    queue->start_processing(0);
  });
  t.detach();
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  queue->stop_processing();
  EXPECT_EQ(delegate->responses->size(),  9 + 8 + 7 + 6 + 5 + 4 + 3 + 2 + 1 + 1);
}

TEST(processingQueueTests, testStopProcessingBeforeStarting)
{
  std::shared_ptr<SpyProcessingQueueDelegate> delegate = std::make_shared<SpyProcessingQueueDelegate>();
  std::shared_ptr<ProcessingQueue<int, int>> queue = std::make_shared<ProcessingQueue<int, int>>();
  queue->delegate = delegate->weak_from_this();
  for(int i = 0; i < 10; i++)
  {
    queue->push(i, 0);
  }

  queue->stop_processing();
  std::thread t([queue](){
    queue->start_processing(0);
  });
  t.detach();
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  queue->stop_processing();
  EXPECT_EQ(delegate->responses->size(), 10);
}

TEST(processingQueueTests, testCantStartProcessingTwice)
{
  std::shared_ptr<SpyProcessingQueueDelegate> delegate = std::make_shared<SpyProcessingQueueDelegate>();
  std::shared_ptr<ProcessingQueue<int, int>> queue = std::make_shared<ProcessingQueue<int, int>>();
  queue->delegate = delegate->weak_from_this();
  for(int i = 0; i < 10; i++)
  {
    queue->push(i, 0);
  }

  std::thread t([queue]()
  {
    queue->start_processing(500);
  });
  t.detach();
  std::thread t2([queue]()
   {
     queue->start_processing(0);
   });
  t2.detach();
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  queue->stop_processing();
  //Checks to see that all 10 were processed which is only possible if
  //the second thread also starts processing from the queue.
  ASSERT_EQ(delegate->responses->size(), 10);
}