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
                SigTermHandler();
                /**
                 * Start listening for sigterm events
                 */
                //void register_sigterm_handlers();
                bool app_running();
                void exit_counter_inc(std::string owner);
                void exit_counter_dec();
            };

        }
    }
}