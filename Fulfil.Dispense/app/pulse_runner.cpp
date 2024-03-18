//
// Created by amber on 8/4/20.
//
//
#include <Fulfil.CPPUtils/networking.h>
#include <Fulfil.CPPUtils/file_system_util.h>
#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.CPPUtils/inih/INIReader.h>
#include <arpa/inet.h>
#include <json.hpp>
using fulfil::utils::Logger;
using fulfil::utils::FileSystemUtil;
/**
 * The first argument should be a file with the json for the request to send. It will send the request and wait for
 * a response
 */
int main(int argc, char** argv)
{
  Logger* test_logger = Logger::Instance((Logger::default_logging_dir +"/test_logs"), "pulse_runner",
                                         Logger::Level::Info,Logger::Level::Info);
  std::string config_section;
  if(argc < 2)
  {
    config_section = "pulse";
  } else {
    config_section = argv[1];
  }
  std::unique_ptr<INIReader> reader = std::make_unique<INIReader>("test_func.ini", true);
  if (reader->ParseError() < 0) {
    test_logger->Fatal("Can't load 'test_func.ini, check path.'\n");
    throw std::runtime_error("Failure to parse ini file..");
  }
  if (reader->Sections().find(config_section) == reader->Sections().end()){
    test_logger->Fatal("The specified config section is not in test_func.ini .\n");
    throw std::invalid_argument("Test config section supplied in command line not in test ini.");
  }
  std::string filename = reader->Get(config_section, "fixture_dir", "Failed to find");
  FileSystemUtil::join_append(filename, "nop_request.json");
  std::string ip_addr =  reader->Get(config_section, "ipAddress","Failed to find");
  int pulse_freq =  reader->GetInteger(config_section, "pulse_log_freq",30);
  int sleep_duration =  reader->GetInteger(config_section, "sleep_duration",20);
  test_logger->Info("Configs from ini:\n\tFilename: {}\n\tIP Address: {}\n\tApprox time between pulse logs: {}",
          filename, ip_addr, pulse_freq*sleep_duration);
  std::shared_ptr<std::string> json = fulfil::utils::FileSystemUtil::get_string_from_file(filename.c_str());
  test_logger->Info("JSON REQUEST:{}", *json);
  fulfil::utils::networking::SocketCommandHeader header;
  test_logger->Info("memcpy for header");
  memcpy(&header.command_id, "012345678901", 12);
  header.bytesleft = json->size();
  test_logger->Info("Header was created");
  int sock0 = 0, valread0;
  int sock1 = 1, valread1;
  struct sockaddr_in serv_addr0;
  struct sockaddr_in serv_addr1;
  char buffer[1024] = {0};
  test_logger->Info("Creating socket");
  if ((sock0 = socket(AF_INET, SOCK_STREAM, 0)) < 0 ||
    (sock1 = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    test_logger->Fatal("\n Socket creation error \n");
    return -1;
  }
  serv_addr0.sin_family = AF_INET;
  serv_addr1.sin_family = AF_INET;
  int port0 = 9500;
  int port1 = 9501;
  serv_addr0.sin_port = htons(port0);
  serv_addr1.sin_port = htons(port1);
  // Convert IPv4 and IPv6 addresses from text to binary form
  test_logger->Info("Checking Address");
  if(inet_pton(AF_INET, ip_addr.c_str(), &serv_addr0.sin_addr)<=0 ||
    inet_pton(AF_INET, ip_addr.c_str(), &serv_addr1.sin_addr)<=0)
  {
    test_logger->Error("\nInvalid address/ Address not supported \n");
    return -1;
  }
  test_logger->Info( "Connecting to socket now");
  if (connect(sock0, (struct sockaddr *)&serv_addr0, sizeof(serv_addr0)) < 0 ||
          connect(sock1, (struct sockaddr *)&serv_addr1, sizeof(serv_addr1)) < 0)
  {
    test_logger->Fatal("\nConnection Failed \n");
    return -1;
  }
  test_logger->Info("Connection Successful");
  try {
    test_logger->Info("Starting pulse");
    uint32_t iters = 0;
    while (true)
    {
      sleep(sleep_duration);
      if (iters % pulse_freq == 0) test_logger->Info("Starting pulse number {}", iters);
      iters ++;
      char* sending_buffer = (char*) malloc(sizeof(fulfil::utils::networking::SocketCommandHeader) + json->size());
      std::memcpy(sending_buffer, &header, sizeof(fulfil::utils::networking::SocketCommandHeader));
      std::memcpy(&sending_buffer[sizeof(fulfil::utils::networking::SocketCommandHeader)], json->c_str(), json->size());
      send(sock0, sending_buffer, sizeof(fulfil::utils::networking::SocketCommandHeader) + json->size(), 0 );
      send(sock1, sending_buffer, sizeof(fulfil::utils::networking::SocketCommandHeader) + json->size(), 0 );
      free(sending_buffer);
      while(true)
      {
        valread0 = read( sock0, buffer, sizeof(fulfil::utils::networking::SocketCommandHeader));
        valread1 = read( sock1, buffer, sizeof(fulfil::utils::networking::SocketCommandHeader));
        if(valread0 != -1)
        {
          fulfil::utils::networking::SocketCommandHeader header2{};
          std::memcpy(&header2, buffer, sizeof(fulfil::utils::networking::SocketCommandHeader));
          char* buff = (char*) malloc(header2.bytesleft);
          memset(buff, 0, header2.bytesleft);
          valread0 = read(sock0, buff, header2.bytesleft);
          free(buff);
        }
        if(valread1 != -1)
        {
          fulfil::utils::networking::SocketCommandHeader header2{};
          std::memcpy(&header2, buffer, sizeof(fulfil::utils::networking::SocketCommandHeader));
          char* buff = (char*) malloc(header2.bytesleft);
          memset(buff, 0, header2.bytesleft);
          valread1 = read(sock1, buff, header2.bytesleft);
          free(buff);
        } if ( valread0 != -1 && valread1 != -1) break;
      }
    }
  } catch (const std::exception& e ) {
    test_logger->Fatal("Caught standard exception in last chance catch or control loop: \n\t{}", e.what());
    return EXIT_FAILURE;
  }
  test_logger->Info("Ending program");
  return EXIT_SUCCESS;
}

