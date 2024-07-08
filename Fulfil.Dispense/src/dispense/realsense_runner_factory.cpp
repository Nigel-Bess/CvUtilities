#include <Fulfil.CPPUtils/logging.h>
#include <experimental/filesystem>
#include <Fulfil.Dispense/dispense/realsense_runner_factory.h>
#include <Fulfil.Dispense/dispense/dispense_manager.h>
#include <Fulfil.Dispense/tray/tray_manager.h>
#include <Fulfil.CPPUtils/inih/INIReader.h>
#include "Fulfil.Dispense/mongo/mongo_tray_calibration.h"
#include <Fulfil.Dispense/recipes/tray_dimensional_configuration.h>

#include <utility>

using fulfil::dispense::RealsenseRunnerFactory;
using fulfil::utils::Logger;
using fulfil::mongo::MongoTrayCalibration;

fulfil::dispense::RealsenseRunnerFactory::RealsenseRunnerFactory(std::shared_ptr<INIReader> reader,
  std::shared_ptr<ff_mongo_cpp::MongoConnection> conn) :
                                             dispense_man_reader{reader}, mongo_conn{conn}
{
  //TODO once dispense manager needs stabilizes, we should pass config info through interface, not config pointer
  std::experimental::filesystem::path config_base = INIReader::get_compiled_default_dir_prefix();
  auto ini_parse_log = [&config_base] (std::string_view filename) {
      Logger::Instance()->Fatal("Failure to load and append tray calibration data. Have you calibrated the cameras yet? Check path. Is {} in directory {}?",  filename, config_base.string());
  };
  std::shared_ptr<INIReader> tray_reader = std::make_shared<INIReader>("tray_config.ini", true);
  fulfil::utils::ini::validate_ini_parse(*tray_reader, "tray_config.ini", ini_parse_log);

  if (tray_reader->GetBoolean("flags", "use_mock_calibration", false)){
      tray_reader->appendReader(INIReader(config_base / "tray_mock_calibration_data.ini", false));
  } else {
      tray_reader->appendReader(INIReader(config_base / "tray_calibration_data_dispense.ini", false));
      tray_reader->appendReader(INIReader(config_base / "tray_calibration_data_hover.ini", false));
  }
  fulfil::utils::ini::validate_ini_parse(*tray_reader, "tray_calibration_data_*.ini", ini_parse_log);
  this->tray_config_reader = std::move(tray_reader);
}

std::shared_ptr<fulfil::dispense::bays::BayRunner> fulfil::dispense::RealsenseRunnerFactory::create(int bay_num,
    std::shared_ptr<fulfil::depthcam::Session> LFB_session, std::shared_ptr<fulfil::depthcam::Session> tray_session)
{
  if (!mongo_conn->IsConnected()){
      Logger::Instance()->Error("Failure to make connection to MongoDB. Ignoring for now...");
      //throw std::runtime_error("Unable to connect to MongoDB");
  }
  fulfil::configuration::tray::TrayDimensions tray_builder =
      fulfil::configuration::tray::set_bay_wide_tray_dimensions(this->tray_config_reader,
               "tray_dimensions_" + this->dispense_man_reader->Get("device_specific", "tray_config_type", "4.1"));

  std::shared_ptr<fulfil::dispense::tray::TrayManager> tray_manager =
      std::make_shared<fulfil::dispense::tray::TrayManager>(tray_session, tray_builder);
    char host[HOST_NAME_MAX];
    gethostname(host, HOST_NAME_MAX);
    std::string bay = bay_num == 0 ? "-A" : "-B";
    if(LFB_session != nullptr)
        LFB_session->set_sensor_name(host + bay + " LFB  cam");
    if(tray_session != nullptr)
        tray_session->set_sensor_name(host + bay + " Tray cam");
  return std::shared_ptr<fulfil::dispense::DispenseManager>(new fulfil::dispense::DispenseManager(bay_num, LFB_session,
    tray_session, this->dispense_man_reader,
          this->tray_config_reader, this->mongo_conn, std::move(tray_manager)));
}

std::shared_ptr<fulfil::dispense::bays::BayRunner> fulfil::dispense::RealsenseRunnerFactory::create_empty(int bay_num)
{
  return std::shared_ptr<fulfil::dispense::DispenseManager>(new fulfil::dispense::DispenseManager(bay_num, nullptr, nullptr,
          nullptr, nullptr, nullptr, nullptr));
}
