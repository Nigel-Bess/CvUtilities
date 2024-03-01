//
// Created by Amber Thomas on 4/17/22.
//
#include <Fulfil.Dispense/recipes/tray_calibration_manager.h>


fulfil::configuration::tray::TrayCalibrations::TrayCalibrations(const Eigen::Affine3d &dispense_transformation,
    const Eigen::Affine3d &hover_transformation,
    const Eigen::Affine3d &tongue_transformation)
    : dispense_camera_to_tray_transformation(dispense_transformation),
      hover_camera_to_tray_transformation(hover_transformation),
      tongue_camera_to_tray_transformation(tongue_transformation) {}

fulfil::configuration::tray::TrayCalibrations
fulfil::configuration::tray::create_calibration_manager(const std::shared_ptr<INIReader>&tray_configs,
  const std::string& sensor_id)
{
    std::shared_ptr<Eigen::Affine3d>
        dispense_transform = create_transformation(tray_configs, sensor_id + "_");
    std::shared_ptr<Eigen::Affine3d>
        hover_transform = create_transformation(tray_configs, sensor_id + "_hover_");
    std::shared_ptr<Eigen::Affine3d>
        tongue_transform = create_transformation(tray_configs, sensor_id + "_tongue_");
    return TrayCalibrations(*dispense_transform, *hover_transform, *tongue_transform);

}


std::shared_ptr<Eigen::Affine3d>
fulfil::configuration::tray::create_transformation(const std::shared_ptr<INIReader>&tray_configs,
         const std::string &coordinate_config_prefix)
{
  std::shared_ptr<Eigen::Matrix3Xd>
      mm_fiducial_coordinates = fulfil::configuration::tray::load_eigen_matrix(tray_configs,
                              coordinate_config_prefix + "_tray_coordinates");

  // These are in depth points correct?
  std::shared_ptr<Eigen::Matrix3Xd>
      camera_fiducial_coordinates = fulfil::configuration::tray::load_eigen_matrix(tray_configs,
                              coordinate_config_prefix + "_camera_coordinates");
  fulfil::depthcam::KabschHelper helper;
  std::shared_ptr<Eigen::Affine3d>
      camera_to_mm_transform = helper.find_translation_between_points(
                          camera_fiducial_coordinates, mm_fiducial_coordinates);
  return camera_to_mm_transform;
}

std::shared_ptr<Eigen::Matrix3Xd>
fulfil::configuration::tray::load_eigen_matrix(const std::shared_ptr<INIReader>&tray_configs,
                                                    const std::string &section) {
  long num_calibration_coordinates = tray_configs->GetInteger(section, "num_calib_coordinates", 4);
  std::vector<double> dims; // num_calib_coordinates x height x width x depth
  dims.reserve(num_calibration_coordinates);
  std::shared_ptr<Eigen::Matrix3Xd> point_coordinates =
      std::make_unique<Eigen::Matrix3Xd>(3, num_calibration_coordinates);

  typedef typename Eigen::Map<Eigen::RowVectorXd, Eigen::Unaligned> RowInitMapXd;
  point_coordinates->col(0) << RowInitMapXd(tray_configs->GetDoubleVector(section, "x").data(),num_calibration_coordinates),
  point_coordinates->col(1) << RowInitMapXd(tray_configs->GetDoubleVector(section, "y").data(),num_calibration_coordinates),
  point_coordinates->col(2) << RowInitMapXd(tray_configs->GetDoubleVector(section, "depth").data(), num_calibration_coordinates);
  return point_coordinates;

}