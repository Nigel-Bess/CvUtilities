#include "Fulfil.Dispense/dispense/test_manager.h"

using fulfil::dispense::TestManager;

int main(int argc, char** argv)
{
    printf("SSSSSTARTING\n");
//   TestManager tm;
  auto tm = std::make_shared<TestManager>();
  tm->bind_delegates();
  tm->start();
  while(true)std::this_thread::sleep_for(std::chrono::milliseconds(1000));

}