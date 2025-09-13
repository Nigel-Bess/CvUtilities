#include <iostream>
#include <Fulfil.CPPUtils/comm/GrpcService.h>
#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.CPPUtils/commands/dispense_command.h>
#include <Fulfil.CPPUtils/orbbec/orbbec_manager.h>
#include "commands/tcs/tcs_perception.h"
#include "commands/tcs/tcs_error_codes.h"
#include "commands/tcs/tcs_response.h"

using fulfil::dispense::commands::tcs::TCSPerception;

int main()
{
    Logger::Instance()->Info("Starting TCS");
    // Open all the grpc ports / "machines" that I own, at the moment, all of them as the TCS monolith!

    // TODO: Start Orbbec-manager and Vimba-managers!
    auto orbbec_manager = new OrbbecManager(Logger::Instance());
    //auto vimba_manager = std::make_shared<VimbaManager>();

    // TODO: stop loop after receiving sigterm!

    orbbec_manager->start_manager();
    Logger::Instance()->Info("TCS Started");

    auto tcs_perception = new TCSPerception();

    // TODO: block on listen instead
    std::this_thread::sleep_for(std::chrono::milliseconds(400000));
}
