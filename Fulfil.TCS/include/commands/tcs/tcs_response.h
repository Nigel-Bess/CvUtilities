//
// Created by Jess on 9/5/24.
//

#ifndef FULFIL_DISPENSE_TCS_RESPONSE_H
#define FULFIL_DISPENSE_TCS_RESPONSE_H

#include <json.hpp>
#include <iostream>
#include <Fulfil.CPPUtils/logging.h>

namespace fulfil::dispense::commands::tcs {
  class TCSResponse final
  {
  private:
    // fields set in the constructor
    std::shared_ptr<std::string> command_id;

    // methods
    void encode_payload();

  public:
    std::shared_ptr<std::string> primary_key_id;
    int success_code;
    std::string error_description;
    bool is_bag_empty;
    bool always_approve_for_release;

    TCSResponse(std::shared_ptr<std::string> command_id,
                                std::shared_ptr<std::string> primary_key_id,
                                bool always_approve_for_release,
                                int success_code,
                                bool is_bag_empty,
                                std::string error_description=std::string(""));

    int get_success_code();
    

    // fields that are set outside of the constructor
    std::shared_ptr<std::string> payload;
  };
} // namespace fulfil::dispense::commands::tcs

#endif //FULFIL_DISPENSE_TCS_RESPONSE_H