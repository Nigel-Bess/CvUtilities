#include "Fulfil.CPPUtils/eigen/matrix3xd_builder.h"
#include <stdexcept>

using fulfil::utils::eigen::Matrix3XdBuilder;

Matrix3XdBuilder::Matrix3XdBuilder()
{
    this->inner_matrix = std::make_shared<Eigen::Matrix3Xd>(3,0);
    this->column_count = 0;
}

Matrix3XdBuilder::Matrix3XdBuilder(int init_size)
{
    this->inner_matrix = std::make_shared<Eigen::Matrix3Xd>(3, init_size);
    this->column_count = init_size;
}

Matrix3XdBuilder::Matrix3XdBuilder(int init_size, float init_x, float init_y, float init_z)
{
    this->inner_matrix = std::make_shared<Eigen::Matrix3Xd>(3, init_size);
    this->column_count = init_size;
    if (init_size > 0){
        this->inner_matrix->row(0) = Eigen::MatrixXd::Constant(1, init_size, init_x);
        this->inner_matrix->row(1) = Eigen::MatrixXd::Constant(1, init_size, init_y);
        this->inner_matrix->row(2) = Eigen::MatrixXd::Constant(1, init_size, init_z);
    }
}
// TODO rename to add_col
void Matrix3XdBuilder::add_row(float x, float y, float z)
{
    this->column_count += 1;
    this->inner_matrix->conservativeResize(3, column_count);
    (*this->inner_matrix)(0, this->column_count - 1) = x;
    (*this->inner_matrix)(1, this->column_count - 1) = y;
    (*this->inner_matrix)(2, this->column_count - 1) = z;
}

void Matrix3XdBuilder::update_row(int index, float x, float y, float z)
{
    if (index >= this->column_count) throw std::out_of_range("Attempted to update an entry outside of the range of matrix builder!");
    (*this->inner_matrix)(0, index) = x;
    (*this->inner_matrix)(1, index) = y;
    (*this->inner_matrix)(2, index) = z;
}

std::shared_ptr<Eigen::Matrix3Xd> Matrix3XdBuilder::get_matrix() const
{
    //Consider returning a copy instead of a reference
    return this->inner_matrix;
}