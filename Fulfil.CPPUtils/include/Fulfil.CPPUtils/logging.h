//
// Created by Amber Thomas on 5/5/20.
//

#ifndef FULFIL_CPPUTILS_LOGGING_H
#define FULFIL_CPPUTILS_LOGGING_H

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

static const std::string log_file_name = "cv_logs";

namespace fulfil::utils
    {
/**
 * Logger class that wraps spdlog so we can swap vendors/ implementation w/o changing
 * code logger is used in.
 * */
        class Logger {
        public:

            /**
             * Restrict log levels to specific types (use SetFileLogLevel or
             * SetConsoleLogLevel to change the level of messages reported by the logger)
             * */
            enum class Level{Trace, Debug, Info, Warn, Error, Fatal, TurnOff};
            const std::string file_log_inst = "rolling_log_inst";
            const std::string console_log_inst = "console_log_inst";

            /**
             *  Log message levels ordered by severity, type handling is forwarded to spdlog
             *
             *  Note: To prevent linking errors when using a veriadic template, function
             *  must be defined in header.
             * */
            template<typename... Args>
            void Trace(Args&& ... args)
            {
              // TODO might want to push trace logger to different sink?
              // log anything without worrying about performance impact, never set in prod
              spdlog::get(console_log_inst)->trace(std::forward<Args>(args)...);
              spdlog::get(file_log_inst)->trace(std::forward<Args>(args)...);
              spdlog::get(file_log_inst)->flush();
            }

            template<typename... Args>
            void Debug(Args&& ... args)
            {
              // might want to log in production when searching for issues
              spdlog::get(console_log_inst)->debug(std::forward<Args>(args)...);
              spdlog::get(file_log_inst)->debug(std::forward<Args>(args)...);
              spdlog::get(file_log_inst)->flush();
            }

            template<typename... Args>
            void Info(Args&& ... args)
            {
              // should log in production
              spdlog::get(console_log_inst)->info(std::forward<Args>(args)...);
              spdlog::get(file_log_inst)->info(std::forward<Args>(args)...);
              spdlog::get(file_log_inst)->flush();
            }

            template<typename... Args>
            void Warn(Args&& ... args)
            {
              // logs issue that might require action
              spdlog::get(console_log_inst)->warn(std::forward<Args>(args)...);
              spdlog::get(file_log_inst)->warn(std::forward<Args>(args)...);
              spdlog::get(file_log_inst)->flush();
            }

            template<typename... Args>
            void Error(Args&& ... args)
            {
              // log issue that requires action
              spdlog::get(console_log_inst)->error(std::forward<Args>(args)...);
              spdlog::get(file_log_inst)->error(std::forward<Args>(args)...);
              spdlog::get(file_log_inst)->flush();
            }

            template<typename... Args>
            void Fatal(Args&& ... args)
            {
              // logs issue that has likely killed application, or is very severe
              spdlog::get(console_log_inst)->critical(std::forward<Args>(args)...);
              spdlog::get(file_log_inst)->critical(std::forward<Args>(args)...);
              spdlog::get(file_log_inst)->flush();
            }

            // Throw LOGGING error specifically (to report and issue with the logger itself)
            void ThrowException(std::string msg);

            // Change the level of messages reported by the file & console logger respectively
            int SetFileLogLevel(Level file_level);
            int SetConsoleLogLevel(Level console_level);
            int SetConsoleLogLevel(const std::string &console_level);
            int SetFileLogLevel(const std::string &file_level);
            // TODO (Amber) add option to push logs to cloud?

            /**
             * To restrict access to the constructor of the logger object (to ensure that there is
             * only a single global logger, the object can only be accessed through an instance).
             * The first time that Instance is called, the necessary parameters to construct the
             * logger must be supplied otherwise an error will be thrown.
             * */

            static Logger* Instance();
            static Logger* Instance(std::string log_dir, std::string file_prefix = log_file_name,
                    Level file_level = Level::Debug, Level console_level = Level::Debug);

            static std::string default_logging_dir;




        private:
            // Protect the constructor of the global logger
            Logger(std::string log_dir, std::string file_prefix,
                   Level file_level, Level console_level);

            // Helper to set the logger level based on Logger::Level enum
            int SetLogLevel(const std::string& logger_name, Level level, bool first_level_set = false);

            // Helper to set the logger level based on logger level string
            int SetLogLevel(const std::string& logger_name, const std::string& level);


            // static pointer to the sole global logger handler
            static Logger* _logger_instance;



        };

    } // namespace fulfil

#endif //FULFIL_CPPUTILS_LOGGING_H
