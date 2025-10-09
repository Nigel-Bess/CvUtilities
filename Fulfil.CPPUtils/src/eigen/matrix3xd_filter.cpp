#include "Fulfil.CPPUtils/eigen/matrix3xd_filter.h"
#include <Fulfil.CPPUtils/eigen/matrix3xd_builder.h>
#include <iostream>

using fulfil::utils::eigen::Matrix3XdFilter;
using std::shared_ptr;
using std::make_shared;
using fulfil::utils::eigen::Matrix3XdBuilder;


Matrix3XdFilter::Matrix3XdFilter(std::shared_ptr<fulfil::utils::eigen::Matrix3dPredicate> predicate)
{
  this->predicate = predicate;
}

std::shared_ptr<Eigen::Matrix3Xd> Matrix3XdFilter::filter(std::shared_ptr<Eigen::Matrix3Xd> matrix)
{
  shared_ptr<Matrix3XdBuilder> builder = make_shared<Matrix3XdBuilder>();
  for(int i = 0; i < matrix->cols(); i++)
  {
    //std::cout << (*matrix)(0,i) << std::endl;
    if(this->predicate->evaluate(matrix->col(i)))
    {

      builder->add_row((*matrix)(0,i), (*matrix)(1,i), (*matrix)(2,i));
    }
  }
  return builder->get_matrix();
}

std::shared_ptr<Eigen::Matrix3Xd> Matrix3XdFilter::filter_side_dispense(std::shared_ptr<Eigen::Matrix3Xd> matrix)
{
    shared_ptr<Matrix3XdBuilder> builder = make_shared<Matrix3XdBuilder>();
    for (int i = 0; i < matrix->cols(); i++)
    {
        if (this->predicate->evaluate_side_dispense(matrix->col(i)))
        {
            builder->add_row((*matrix)(0, i), (*matrix)(1, i), (*matrix)(2, i));
        }
    }
    return builder->get_matrix();
}

std::shared_ptr<Eigen::Matrix3Xd> Matrix3XdFilter::filter_side_dispense_point_cloud_outside_cavity(std::shared_ptr<Eigen::Matrix3Xd> matrix)
{
    shared_ptr<Matrix3XdBuilder> builder_point_cloud_outside_cavity = make_shared<Matrix3XdBuilder>();
    for (int i = 0; i < matrix->cols(); i++)
    {
        if (!this->predicate->evaluate_side_dispense(matrix->col(i)))
        {
            builder_point_cloud_outside_cavity->add_row((*matrix)(0, i), (*matrix)(1, i), (*matrix)(2, i));
        }
    }
    return builder_point_cloud_outside_cavity->get_matrix();
}