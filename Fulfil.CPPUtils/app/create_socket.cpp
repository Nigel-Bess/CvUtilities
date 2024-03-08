#include <memory>
#include <iostream>
#include "../include/Fulfil.CPPUtils/networking/socket_information.h"
#include "../include/Fulfil.CPPUtils/networking/socket_manager.h"
#include "../include/Fulfil.CPPUtils/networking/socket_network_manager.h"

class SpyCommandParser : public fulfil::utils::networking::SocketCommandParser<int>
{
  int parse_payload(std::shared_ptr<std::string> payload, std::shared_ptr<std::string> request_id) override
  {
    char* buffer = new char(sizeof(int));
    std::memcpy(buffer, payload->c_str(), sizeof(int));
    int request = (int) *buffer;
    return request;
  }
};

class SpyDelegate : public fulfil::utils::networking::SocketNetworkManagerDelegate<int>,
    public std::enable_shared_from_this<fulfil::utils::networking::SocketNetworkManagerDelegate<int>>
{
  void did_receive_request(int request) override
  {
    std::cout << "received valid request " << request << std::endl;
  }
  void did_receive_invalid_request(std::shared_ptr<std::string> request_id) override
  {
    std::cout << "received invalid request with id: " << *request_id << std::endl;
  }
  void did_receive_invalid_request() override
  {
    std::cout << "received invalid request without id" << std::endl;
  }
};

 class SimpleResponse : public fulfil::utils::networking::SocketResponse
 {
   int header_size() override
   {
      return sizeof(fulfil::utils::networking::SocketCommandHeader);
   }
   void* header() override
   {
      fulfil::utils::networking::SocketCommandHeader* header = new fulfil::utils::networking::SocketCommandHeader;
      std::memcpy(&header->command_id, "123456789012", 12);
      header->bytesleft = std::strlen("sample response");
      return header;
   }
   int payload_size() override
   {
      return std::strlen("sample response");
   }
   void* payload() override
   {
      char* payload = new char(std::strlen("sample response"));
      std::memcpy(payload,  "sample response", std::strlen("sample response"));
      return payload;
   }
 };

/**
 * This executable creates a server socket that takes in requests with
 * an int payload. The executable should be passed the port for the socket
 * to use as the command line argument.
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char** argv)
{
  if(argc < 2)
  {
    throw std::invalid_argument("invalid port");
  }
  unsigned short port = std::stoi(argv[1]);
  std::cout << "creating socket on port " << port << std::endl;

  std::shared_ptr<fulfil::utils::networking::SocketInformation> socket_info = std::make_shared<fulfil::utils::networking::SocketInformation>(port);

  std::shared_ptr<fulfil::utils::networking::SocketManager> manager = std::make_shared<fulfil::utils::networking::SocketManager>(socket_info);

  std::shared_ptr<fulfil::utils::networking::SocketCommandParser<int>> parser = std::make_shared<SpyCommandParser>();

  std::shared_ptr<SpyDelegate> delegate = std::make_shared<SpyDelegate>();

  std::shared_ptr<fulfil::utils::networking::SocketNetworkManager<int>> network_manager =
      std::make_shared<fulfil::utils::networking::SocketNetworkManager<int>>(manager, parser);

  network_manager->delegate = delegate->weak_from_this();

  network_manager->start_listening();
  std::this_thread::sleep_for(std::chrono::milliseconds(10000));

  network_manager->send_response(std::make_shared<SimpleResponse>());

  network_manager->stop_listening();

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  return 0;
}