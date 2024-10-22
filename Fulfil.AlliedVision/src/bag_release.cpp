#include <iostream>
#include "vmb-manager.h"

using namespace VmbCPP;

int main()
{
    std::map<int, std::string> bays;
    auto env = getenv("STORE");
    std::string store("PLM");
    if(env != nullptr){
        bays.emplace(1, "192.168.1.163");
    }
    else{
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
    auto log = fulfil::utils::Logger::Instance();
    auto man = new VmbManager(bays, log);

}