//
// Created by vincent on 09/05/22.
//

#ifndef CODE_BIGQUERY_UPLOAD_H
#define CODE_BIGQUERY_UPLOAD_H
#include <json.hpp>

namespace fulfil {
    namespace depthcam {
        namespace data {
            class BQUpload {
            public:
                int upload_traycount_record(nlohmann::json validation_request, nlohmann::json validation_response,
                                            std::string vls_name, std::string calibration);
                int upload_record(const std::string table_name, nlohmann::json data);
            };
        }
    }
}
#endif //CODE_BIGQUERY_UPLOAD_H
