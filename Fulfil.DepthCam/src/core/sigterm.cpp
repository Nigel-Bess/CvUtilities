#include <Fulfil.DepthCam/core/sigterm.h>

// A dirt cheap static class that tracks whether a termination signal has been sent to the app,
// the status can be queried by looking at "fulfil::depthcam::sigterm::app_running"

using fulfil::utils::Logger;
using fulfil::depthcam::sigterm::SigTermHandler;
using fulfil::depthcam::sigterm::_exit_countdown;
using fulfil::depthcam::sigterm::_app_running;

static void sig_handler(int sig_no){
    Logger::Instance()->Info("Graceful shutdown requested (code={})", sig_no);
    _app_running->store(false);
    // Sleep to let other services see that it's shutdown time
    // and wrap up pending requests.
    // Not the best method but it works for dispense
    int last_reported = 99999;
    do {
        if (last_reported != (int)_exit_countdown->load()) {
            Logger::Instance()->Info("Shutdown blocked on {} active threads", _exit_countdown->load());
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        last_reported = _exit_countdown->load();
    } while (_exit_countdown->load() > 0);
    Logger::Instance()->Info("Shut down");
    exit(0);
}

void SigTermHandler::register_sigterm_handlers() {
    // Apply the perfect ctrl+C teardown logic to the K8s-friendly SIGTERM signal as well so the entirety of this service will be Kubernetes friendly for the whole lifecycle
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
    signal(SIGSEGV, sig_handler);
}
