#ifndef FULFIL_CPPUTILS_MATRIX_BUILDER_H
#define FULFIL_CPPUTILS_MATRIX_BUILDER_H

#include <memory>
#include <eigen3/Eigen/Geometry>

namespace fulfil
{
namespace utils
{
namespace eigen
{
/**
 * The purpose of this class is to allow easy
 * creation of dynamically sized matrices of
 * 3d points.
 */
class Matrix3XdBuilder
{
 private:
  /**
   * The inner matrix which is a snapshot of the
   * data that has been currently added through
   * the add_row function.
   */
  std::shared_ptr<Eigen::Matrix3Xd> inner_matrix;
  /**
   * A count of the current number of columns stored
   * in the inner matrix.
   */
  int column_count;
 public:
  /**
   * Matrix3XdBuilder Constructor
   * Initializes the matrix that is being build to one
   * that has 0 columns.
   */
  Matrix3XdBuilder();

  Matrix3XdBuilder(int init_size);

  Matrix3XdBuilder(int init_size, float init_x, float init_y, float init_z);
  /**
   * Adds a row to the matrix that is being build with
   * the given x, y, and z values.
   * @param x the x value of the coordinate.
   * @param y the y value of the coordinate.
   * @param z the z value of the coordinate.
   */
  void add_row(float x, float y, float z);


  void update_row(int index, float x, float y, float z);
  /**
   * Returns the matrix that is being built by this
   * object
   * @return a pointer to the matrix being built
   * by this object
   * @note this is a pointer, not a copy, if the
   * pointer is mutated, the mutation will be seen
   * in every subsequent call to this function.
   * Additionally if a row is added with this builder,
   * it will impact all pointers returned by this
   * function.
   */
  std::shared_ptr<Eigen::Matrix3Xd> get_matrix() const;
};

} // namespace eigen
} // namespace utils
} // namespace fulfil


#endif
