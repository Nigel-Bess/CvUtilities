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
        bays.emplace(5, "000A470DCE5D");//port 8
        bays.emplace(4, "000A4734C113");//port 6
        bays.emplace(3, "000A47186BE8");//port 4
        bays.emplace(2, "000A47044EFC");//port 2
    }
    auto log = fulfil::utils::Logger::Instance();
    auto man = new VmbManager(bays, log);

}