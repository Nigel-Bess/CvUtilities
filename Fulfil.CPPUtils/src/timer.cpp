//
// Created by amber on 12/8/20.
//
#include "Fulfil.CPPUtils/timer.h"
#include "Fulfil.CPPUtils/logging.h"
#include <chrono>
#include <iostream>
#include <sstream>
#include <Fulfil.CPPUtils/date.h>

using fulfil::utils::Logger;

std::string fulfil::utils::timing::return_current_time_ms_and_date()
{
    return fulfil::utils::timing::time_and_date_ms_precision(std::chrono::system_clock::now());
}

std::string fulfil::utils::timing::time_and_date_ms_precision(std::chrono::time_point <std::chrono::high_resolution_clock> tp)
{
    using namespace std::chrono;
    std::time_t current_time = system_clock::to_time_t(tp);
    std::time(&current_time); // Not portable off linux
    tp = time_point_cast<milliseconds>(tp + std::chrono::seconds(std::localtime(&current_time)->tm_gmtoff)) ;
    return date::format("%D %T", tp).substr(0, 21);
}



fulfil::utils::timing::Timer::Timer() :
        m_use_fulfil_logger{true}, m_timer_name{"Scoped"},
            start{std::chrono::high_resolution_clock::now()} {};


fulfil::utils::timing::Timer::Timer(const std::string &timer_name, bool use_fulfil_logger)
{
  this->m_use_fulfil_logger = use_fulfil_logger;
  this->m_timer_name = timer_name;
  this->start = std::chrono::high_resolution_clock::now();
}
fulfil::utils::timing::Timer::~Timer()
{
  this->end = std::chrono::high_resolution_clock::now();
  this->scope_execution_time = this->end - this->start;
  const float ms = this->scope_execution_time.count() * 1000.0F;
  std::stringstream message;
  message << "Timer Complete!  " << this->m_timer_name << " took " << ms << "ms, starting at " << time_and_date_ms_precision(this->start);
  if (this->m_use_fulfil_logger) {
    Logger::Instance()->Debug("{}", message.str());
  } else {
    std::cout << message.str() << '\n';
  }
}