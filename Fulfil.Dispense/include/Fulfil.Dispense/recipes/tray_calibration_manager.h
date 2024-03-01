//
// Created by Amber Thomas on 4/16/22.
//

#ifndef FULFIL_DISPENSE_TRAY_CALIBRATION_MANAGER_H
#define FULFIL_DISPENSE_TRAY_CALIBRATION_MANAGER_H
#include <Fulfil.CPPUtils/inih/INIReader.h>
#include<Fulfil.DepthCam/aruco/kabsch_helper.h>

#include <memory>
#include <eigen3/Eigen/Geometry>

namespace fulfil::configuration::tray {
    class TrayCalibrations {
    private:
        Eigen::Affine3d dispense_camera_to_tray_transformation;
        Eigen::Affine3d hover_camera_to_tray_transformation;
        Eigen::Affine3d tongue_camera_to_tray_transformation;


    public:
      TrayCalibrations()=default;
      explicit TrayCalibrations(
          const Eigen::Affine3d& dispense_transformation,
          const Eigen::Affine3d &hover_transformation,
          const Eigen::Affine3d &tongue_transformation);

      template <typename BasicStrType>
      const Eigen::Affine3d &operator[] (BasicStrType key) const {
          if (key == "Dispense") return dispense_camera_to_tray_transformation;
          if (key == "Hover") return hover_camera_to_tray_transformation;
          if (key == "Tongue") return tongue_camera_to_tray_transformation;
      }

    };
}

namespace fulfil::configuration::tray {
// Temporary until I can abstract the config object
TrayCalibrations
create_calibration_manager(const std::shared_ptr<INIReader> &tray_configs, const std::string& sensor_id);

std::shared_ptr<Eigen::Affine3d>
create_transformation(const std::shared_ptr<INIReader> &tray_configs,
                      const std::string &coordinate_config_prefix);

std::shared_ptr<Eigen::Matrix3Xd>
load_eigen_matrix(const std::shared_ptr<INIReader> &tray_configs,
                  const std::string &section);

}



#endif // FULFIL_DISPENSE_TRAY_CALIBRATION_MANAGER_H
