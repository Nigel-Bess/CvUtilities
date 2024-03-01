//
// Created by amber on 4/26/21.
//

#ifndef FULFIL_CPPUTILS_INIEIGENMATRIXLOADER_H
#define FULFIL_CPPUTILS_INIEIGENMATRIXLOADER_H

#include <Fulfil.CPPUtils/eigen/EigenMatrixLoader.h>
#include <Fulfil.CPPUtils/inih/INIReader.h>

namespace fulfil {
    namespace utils {
        namespace eigen {
            class IniEigenMatrixLoader : public EigenMatrixLoader{
            private:
                std::shared_ptr<INIReader> reader;
                std::string section_prefix;
            public:
                IniEigenMatrixLoader(const std::shared_ptr<INIReader>& reader, const std::string& section_prefix);
                void load_eigen_matrix(std::shared_ptr<Eigen::Matrix3Xd> matrix, const std::string& section) override;

            };
        }
    }
}
#endif //FULFIL_CPPUTILS_INIEIGENMATRIXLOADER_H
