
#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_DISPENSE_REQUEST_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_DISPENSE_REQUEST_H_
#include <memory>
#include <Fulfil.Dispense/commands/dispense_request.h>
#include <Fulfil.Dispense/commands/dispense_request_delegate.h>
#include <Fulfil.Dispense/commands/dispense_response.h>

namespace fulfil::dispense::commands
{
/**
 * The purpose of this class is to outline the general
 * required functionality for any type of dispense command.
 */
class DispenseRequest
{
 public:
  /**
   * Executes the command using the delegate to provide
   * functionality when necessary.
   * @return the result of executing the command.
   */
  virtual std::shared_ptr<fulfil::dispense::commands::DispenseResponse> execute() = 0;

  /**
   * String with the id for the command.
   */
  std::shared_ptr<std::string> request_id;

  /**
  * ID for data logging purposes
  */
  std::shared_ptr<std::string> PrimaryKeyID;

  /**
   * A weak pointer to the delegate that contains all of the
   * necessary information to process the commands. This is following
   * the visitor pattern. The general flow will be that the command
   * will know what type of command it is and be able to call the
   * appropriate method on the delegate and pass the required information
   * to successfully complete the request.
   */
  std::weak_ptr<fulfil::dispense::commands::DispenseRequestDelegate> delegate;
};

} // namespace fulfil::dispense::commands


#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_DISPENSE_REQUEST_H_
