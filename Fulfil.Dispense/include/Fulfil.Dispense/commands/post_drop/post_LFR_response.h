//
// Created by steve on 2/25/21.
//

#ifndef FULFIL_DISPENSE_SRC_COMMANDS_POST_DROP_POST_DROP_RESPONSE_H_
#define FULFIL_DISPENSE_SRC_COMMANDS_POST_DROP_POST_DROP_RESPONSE_H_
#include <Fulfil.Dispense/commands/dispense_response.h>
#include <Fulfil.CPPUtils/logging.h>

namespace fulfil
{
namespace dispense {
namespace commands
{
/**
 * The purpose of this class is to outline and contain
 * all of the information required for the response to
 * a drop target response
 */
class PostLFRResponse final : public fulfil::dispense::commands::DispenseResponse
{
 private:

  /**
   * The id for the request.
   */
  std::shared_ptr<std::string> command_id;

  /**
   * depth of tallest item in LFB bag, in mm, relative to the top surface of the bag
   */
  float max_Z;
  /**
   *  success_code = 0 if successful, > 0 if there was an error
   */
  int success_code;
  /**
   *  code for the amount of dispensed items detected in bag. Valid values: -1 (error), 0, or 1 (>= 0)
   * */
  int items_dispensed;

  /**
   * The payload to be sent in response to the request
   */
  std::shared_ptr<std::string> payload;

  /**
   * Detected percentage of the bag that is full of items, based on depth stream
  */
  int Bag_Full_Percent;
  /**
   * Detected percentage of the detected dispensed item that landed in the provided target region
   */
  int Item_On_Target_Percent;

  /**
   * List of products IDs that must be overflowed to another bag, will no longer fit in current bag based on post-image analysis
   */
   std::vector<int> Products_To_Overflow;

  /**
   * Encodes the payload in the payload string variable on this object.
   */
  void encode_payload();
 public:

  /**
   * constructor that initializes a response indicating a success with additional parameters to be sent
   */
  PostLFRResponse(std::shared_ptr<std::string> command_id, int success_code, float max_Z = 0, int items_dispensed = -1,
                   int Bag_Full_Percent = 0, int Item_On_Target_Percent = 0);
  /**
   * Returns the command id for the response.
   * @return pointer to string containing command id for the response.
   */
  std::shared_ptr<std::string> get_command_id() override;

  int get_success_code();

  void set_items_dispensed(std::pair<int, int> values);

  void set_products_to_overflow(std::vector<int> products);

  /**
   * Returns the size (in bytes) of the payload containing information about the drop result.
   * @return size (in bytes) of the payload to be sent.
   */
  int dispense_payload_size() override;
  /**
   * Returns the payload containing information about the drop result.
   * @return pointer to string containing data representing the drop result.
   */
  std::shared_ptr<std::string> dispense_payload() override;
};
} // namespace commands
} // namespace dispense
} // namespace fulfil

#endif //FULFIL_DISPENSE_SRC_COMMANDS_POST_DROP_POST_DROP_RESPONSE_H_
