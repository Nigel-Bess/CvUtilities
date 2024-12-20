#include <iostream>
#include "vmb-manager.h"

using namespace VmbCPP;

int main()
{
    auto facility = getenv("FACILITY_IDENTIFIER");
    std::string facilityStr = std::string(facility);
    std::map<int, std::string> bays;

    // In a perfect world this mapping wouldn't be hardcoded, but Repack should have limited usage, soooo...
    if(facility == nullptr){
        printf("Adding 1 dev mode camera");
        bays.emplace(1, "192.168.1.163");
    }
    if (facilityStr == "pioneer") {
        printf("Adding 1 Pioneer camera");
        bays.emplace(1, "10.10.10.40");
    }
    else if (facilityStr == "plm") {
        printf("Adding 11 PLM cameras");
        bays.emplace(1, "000A47379C57");
        bays.emplace(2, "000A47044EFC");
        bays.emplace(3, "000A47186BE8");
        bays.emplace(4, "000A4734C113");
        bays.emplace(5, "000A470DCE5D");
        bays.emplace(6, "000A4722B40D");
        bays.emplace(7, "000A47127BB4");
        bays.emplace(8, "000A470450A5");
        bays.emplace(9, "000A4714F7F5");
        bays.emplace(10, "000A47365B46");
        bays.emplace(11, "000A4739DA62");
    }
    else if (facilityStr == "whisman") {
        printf("Whisman not supported, pausing for 1 hour before exit");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 60 * 60));
        exit(1);
    }
    else {
        printf("No valid FACILITY_IDENTIFIER set! Exiting! %s", facility);
        std::this_thread::sleep_for(std::chrono::milliseconds(10000));
        exit(1);
    }
    auto log = fulfil::utils::Logger::Instance();
    auto man = new VmbManager(bays, log);

}