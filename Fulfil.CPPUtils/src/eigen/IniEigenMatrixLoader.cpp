//
// Created by amber on 4/26/21.
//
#include <Fulfil.CPPUtils/eigen/IniEigenMatrixLoader.h>

using fulfil::utils::eigen::IniEigenMatrixLoader;

IniEigenMatrixLoader::IniEigenMatrixLoader(const std::shared_ptr<INIReader>& reader, const std::string& section_prefix)
{
  this->reader = reader;
  this->section_prefix = section_prefix;
}

void IniEigenMatrixLoader::load_eigen_matrix(std::shared_ptr<Eigen::Matrix3Xd> matrix, const std::string& section)
{
  std::vector<float> dims; // num_calib_coordinates x height x width x depth
  std::string full_section_name = this->section_prefix.append(section);
  this->reader->FillFloatVector(full_section_name, "x", dims);
  int num_points_x = dims.size();
  for (int i = 0; i < num_points_x; i++)
    (*matrix)(0, i) = dims[i];
  dims.clear();
  this->reader->FillFloatVector(full_section_name, "y", dims);
  int num_points_y = dims.size();
  if (num_points_x != num_points_y) throw std::runtime_error("Mismatch in number of Eigen Matrix columns per dimension!");
  for (int i = 0; i < num_points_y; i++)
    (*matrix)(1, i) = dims[i];
  dims.clear();
  this->reader->FillFloatVector(full_section_name, "depth", dims);
  int num_points_z = dims.size();
  if (num_points_x != num_points_z) throw std::runtime_error("Mismatch in number of Eigen Matrix columns per dimension!");
  for (int i = 0; i < num_points_z; i++)
    (*matrix)(2, i) = dims[i];
}