//
// Created by amber on 12/8/20.
//

#ifndef FULFIL_DISPENSE_TIMER_H
#define FULFIL_DISPENSE_TIMER_H
#include <chrono>
#include <string>

using namespace std::chrono;

namespace fulfil::utils::timing {// Timer class to log absolute time until complete for a scope.
    std::string time_and_date_ms_precision(std::chrono::time_point <std::chrono::high_resolution_clock> tp);
    std::string return_current_time_ms_and_date();

    class Timer {
      private:
        std::chrono::time_point <std::chrono::high_resolution_clock> start, end;
        std::chrono::duration<float> scope_execution_time;
        bool m_use_fulfil_logger;
        std::string m_timer_name;
      public:
        Timer();
        Timer(const std::string &timer_name, bool use_fulfil_logger=true);
        ~Timer();

    };
}// namespace fulfil::utils::timing

inline std::chrono::system_clock::time_point CurrentTime() {
    return std::chrono::system_clock::now();
}

inline double ms_elapsed(std::chrono::system_clock::time_point start) {
        auto elapsed = duration_cast<std::chrono::milliseconds>(CurrentTime() - start).count();
        return static_cast<double>(elapsed);   
    }

#endif //FULFIL_DISPENSE_TIMER_H
