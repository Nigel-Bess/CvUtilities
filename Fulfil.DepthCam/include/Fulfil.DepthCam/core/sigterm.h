#include <signal.h>
#include <cstring>
#include <thread>
#include <Fulfil.CPPUtils/logging.h>

// A dirt cheap static class that tracks whether a termination signal has been sent to the app,
// the status can be queried by looking at "fulfil::depthcam::sigterm::app_running()"

using fulfil::utils::Logger;

namespace fulfil
{
    namespace depthcam
    {
        namespace sigterm
        {

            class SigTermHandler
            {
            public:
                SigTermHandler() {};
                /**
                 * Start listening for sigterm events
                 */
                void register_sigterm_handlers();
            };

            // Static sig handler state should only be accessed by sigterm.cpp
            static std::atomic<bool>* _app_running = new std::atomic<bool>(true);
            static std::atomic<int>* _exit_countdown = new std::atomic<int>(0);
            static SigTermHandler* _instance = nullptr;

            // Call this to kick off sigterm handling logic, poll
            // app_running() to see if the app is intended to be in
            // a running or graceful shutdown state
            static SigTermHandler* register_handler()
            {
                if (_instance == nullptr)
                {
                    _instance = new SigTermHandler();
                    _instance->register_sigterm_handlers();
                }
                return _instance;
            }

            static bool app_running()
            {
                return _app_running->load();
            }

            static void exit_counter_inc(std::string owner)
            {
                Logger::Instance()->Info("Graceful exit will wait for {}", owner);
                _exit_countdown->fetch_add(1);
            }

            static void exit_counter_dec()
            {
                _exit_countdown->fetch_sub(1);
            }

        }
    }
}