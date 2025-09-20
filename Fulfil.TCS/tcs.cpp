#include <iostream>
#include <Fulfil.CPPUtils/comm/GrpcService.h>
#include <Fulfil.CPPUtils/logging.h>
//#include <Fulfil.CPPUtils/commands/dispense_command.h>
#include <Fulfil.CPPUtils/orbbec/orbbec_manager.h>
#include "commands/tcs/tcs_perception.h"
#include "commands/tcs/tcs_controller.h"
#include "commands/tcs/tcs_error_codes.h"
#include "commands/tcs/tcs_response.h"
#include <signal.h>

using fulfil::dispense::commands::tcs::TCSPerception;
using fulfil::dispense::commands::tcs::TCSController;

static TCSController* tcs_controller;
static OrbbecManager* orbbec_manager;

struct sigaction tcs_manager_destructor;

void tcs_sig_handler(int sig_no){
    tcs_controller->stop_listening();
    orbbec_manager->stop_manager();
}

int main()
{
    Logger::Instance()->Info("Starting TCS");
    // Open all the grpc ports / "machines" that I own, at the moment, all of them as the TCS monolith!

    // TODO: Start Orbbec-manager and Vimba-managers!
    orbbec_manager = new OrbbecManager(Logger::Instance());
    //auto vimba_manager = std::make_shared<VimbaManager>();

    // TODO: stop loop after receiving sigterm!

    orbbec_manager->start_manager();
    Logger::Instance()->Info("TCS Started");

    auto tcs_perception = new TCSPerception();
    tcs_controller = new TCSController(tcs_perception, orbbec_manager);

    // Apply the perfect ctrl+C teardown logic to the K8s-friendly SIGTERM signal as well so the entirety of this service will be Kubernetes friendly for the whole lifecycle
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = &tcs_sig_handler;
    sigaction(SIGINT, &action, &tcs_manager_destructor);
    sigaction(SIGTERM, &action, &tcs_manager_destructor);

    tcs_controller->start_listening();

}
