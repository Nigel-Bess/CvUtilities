//
// Created by amber on 4/26/21.
//

#ifndef FULFIL_CPPUTILS_EIGENMATRIXLOADER_H
#define FULFIL_CPPUTILS_EIGENMATRIXLOADER_H

#include <memory>
#include <eigen3/Eigen/Geometry>

namespace fulfil {
    namespace utils {
        namespace eigen {
            class EigenMatrixLoader {
            public:
                virtual void load_eigen_matrix(std::shared_ptr<Eigen::Matrix3Xd> matrix, const std::string& section) = 0;

            };
        }
    }
}
#endif //FULFIL_CPPUTILS_EIGENMATRIXLOADER_H
