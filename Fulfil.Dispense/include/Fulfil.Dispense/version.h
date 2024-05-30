//
// Copyright (c) 2v1.0.3220 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_VERSION_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_VERSION_H_

// Values will be replaced at build time by the CI/CD pipeline
#ifndef FW_VERSION
#define FW_VERSION "0"
#endif

#ifndef BUILD_DATE
#define BUILD_DATE "unknown"
#endif

#ifndef DISPENSE_COMMIT
#define DISPENSE_COMMIT "unknown"
#endif

#ifndef IS_SOURCE_REPO_CLEAN
#define IS_SOURCE_REPO_CLEAN "unknown"
#endif

namespace versions {
    inline int dispense_api_build_info()
    {
        std::cout << "Latest FW Tag found: " FW_VERSION << ". Build generated on " << BUILD_DATE << ".\n";
        std::cout << "Dispense API commit details: " << DISPENSE_COMMIT << " (" << IS_SOURCE_REPO_CLEAN << ")" << std::endl;
        return 0;
    }
}
#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_VERSION_H_
