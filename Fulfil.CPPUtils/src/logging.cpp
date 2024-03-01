//
// Created by Amber Thomas on 5/5/20.
//



#include <iostream>

#include "Fulfil.CPPUtils/file_system_util.h"
#include "Fulfil.CPPUtils/logging.h"


using fulfil::utils::FileSystemUtil;
using fulfil::utils::Logger;



// Logger instance handling
Logger* Logger::_logger_instance = 0;

//access the default logging dir so file prefix can be changed at first initialization
std::string Logger::default_logging_dir=DEFAULT_FULFIL_LOGGING_DIR;

Logger* Logger::Instance(){
  if (_logger_instance == 0){
    try{
      std::cout << "Attempting to use default logging directory at " << DEFAULT_FULFIL_LOGGING_DIR << std::endl;
      Logger* _logger_instance = Instance(DEFAULT_FULFIL_LOGGING_DIR);
      return _logger_instance;
    } catch (...) {
      throw std::invalid_argument("Issue creating first logging instance with default parameters "
                                  "[using Logger::Instance()] check that cmake properly defined logging directory.");
    }

  }
  return _logger_instance;
}


Logger* Logger::Instance(std::string log_dir,
                         std::string file_prefix,
                         Logger::Level file_level,
                         Logger::Level console_level){
  if (_logger_instance == 0){
    try{
      _logger_instance = new Logger(log_dir, file_prefix, file_level, console_level);
    } catch (spdlog::spdlog_ex& e) {
      const char *exp_err_msg = "logger with name";
      if (strncmp(exp_err_msg, e.what(), strlen(exp_err_msg)) == 0) {
        spdlog::error("Error initializing logger, likely due to thread race. Trying again...");
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        return Instance(log_dir, file_prefix, file_level, console_level);
      } else {
        throw spdlog::spdlog_ex(fmt::format("Error trying to create log:\n{}", e.what()));
      }
    }
  } else {
    _logger_instance->Debug("You attempted to reconstruct a logger that was already instantiated. The existing logger was returned with no changes made to it's settings.");
  }
  
  return _logger_instance;
}

void Logger::ThrowException(std::string msg)
{
  throw spdlog::spdlog_ex(msg);
}
int Logger::SetFileLogLevel(Logger::Level file_level)
{
  return SetLogLevel("log_inst_daily_file", file_level);
}

int Logger::SetConsoleLogLevel(Logger::Level console_level)
{
  return SetLogLevel("log_inst_console", console_level);
}

int Logger::SetFileLogLevel(const std::string& file_level)
{
  return SetLogLevel("log_inst_daily_file", file_level);
}

int Logger::SetConsoleLogLevel(const std::string& console_level)
{
  return SetLogLevel("log_inst_console", console_level);
}


// Private methods implementation

Logger::Logger(std::string log_dir, std::string file_prefix, Logger::Level file_level, Logger::Level console_level){
  // Setup the logging folder, if it does not already exist
  if (!FileSystemUtil::directory_exists(log_dir.c_str()))
  {
    FileSystemUtil::create_directory(log_dir.c_str());
    if(!FileSystemUtil::directory_exists(log_dir.c_str()))
      throw spdlog::spdlog_ex("Failed to create log directory. Exiting main...");
  }
  char hostname[HOST_NAME_MAX];
  gethostname(hostname, HOST_NAME_MAX);

  // TODO add release version to pattern (and verify that trace doesn't slow down code too much)
  std::string prefix_pattern = "[%m-%dT%H:%M:%S.%e] [%^-%l-%$] [%t] %v";

  // Create a daily logger - a new file is created every day at 11:59pm
  std::string log_file_path = log_dir + "/" + file_prefix + ".log";

  auto daily_logger = spdlog::daily_logger_mt("log_inst_daily_file", log_file_path, 23, 59);
  daily_logger->set_pattern(prefix_pattern);
  SetLogLevel("log_inst_daily_file", file_level, true);

  // Create console logger
  auto console_logger = spdlog::stdout_color_mt("log_inst_console");
  console_logger->set_pattern(prefix_pattern);
  SetLogLevel("log_inst_console", console_level, true);


}

int Logger::SetLogLevel(const std::string& logger_name, Logger::Level level, bool first_level_set)
{
  switch(level){
    case Logger::Level::Trace:
      spdlog::get(logger_name)->set_level(spdlog::level::trace);
      break;
    case Logger::Level::Debug:
      spdlog::get(logger_name)->set_level(spdlog::level::debug);
      break;
    case Logger::Level::Info:
      spdlog::get(logger_name)->set_level(spdlog::level::info);
      break;
    case Logger::Level::Warn:
      spdlog::get(logger_name)->set_level(spdlog::level::warn);
      break;
    case Logger::Level::Error:
      spdlog::get(logger_name)->set_level(spdlog::level::err);
      break;
    case Logger::Level::Fatal:
      spdlog::get(logger_name)->set_level(spdlog::level::critical);
      break;
    case Logger::Level::TurnOff:
      // When turnoff is set, nothing will be logged
      spdlog::get(logger_name)->set_level(spdlog::level::off);
      break;
    default:
      if (first_level_set) {
        this->Warn("Attempted to create logger with invalid level enum for {} reporting, using default Debug level.",
                   (logger_name == "log_inst_console" ? "console" : "file"));
        spdlog::get(logger_name)->set_level(spdlog::level::debug);
      } else {
        this->Warn("Attempted to change {} logger level with invalid level enum, no change made to current level.",
                   (logger_name == "log_inst_console" ? "console" : "file"));
      }
      return 1;
  }
  return 0;
}

int Logger::SetLogLevel(const std::string& logger_name, const std::string& level) {

  if (level == "TRACE")
    spdlog::get(logger_name)->set_level(spdlog::level::trace);
  else if (level == "DEBUG")
    spdlog::get(logger_name)->set_level(spdlog::level::debug);
  else if (level == "INFO")
    spdlog::get(logger_name)->set_level(spdlog::level::info);
  else if (level == "WARN")
    spdlog::get(logger_name)->set_level(spdlog::level::warn);
  else if (level == "ERROR")
    spdlog::get(logger_name)->set_level(spdlog::level::err);
  else if (level == "FATAL")
    spdlog::get(logger_name)->set_level(spdlog::level::critical);
  else if (level == "TURN_OFF")
    // When turnoff is set, nothing will be logged
    spdlog::get(logger_name)->set_level(spdlog::level::off);
  else {
    // note that Logger::Instance() does not take string level as args, so this can't be used to set initial log level
    this->Warn(
            "Attempted to change {} logger with invalid input level string '{}'. Please choose from the following levels:\n\t"
            "TRACE, DEBUG, INFO, WARN, ERROR, FATAL, TURN_OFF\nNo change made to current log level.",
            (logger_name == "log_inst_console" ? "console" : "file"), level);
    return 1;
  }
  return 0;
}


