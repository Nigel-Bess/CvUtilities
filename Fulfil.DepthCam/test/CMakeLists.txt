if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    # Setup a macro to easily add new tests to be run by GoogleTest
    macro(package_add_test TESTNAME)
        add_executable(${TESTNAME} ${ARGN})
        target_link_libraries(${TESTNAME} Fulfil.DepthCam gtest gmock_main gtest_main pthread)
        add_test(NAME ${TESTNAME} COMMAND ${TESTNAME})
        set_target_properties(${TESTNAME} PROPERTIES FOLDER test)
    endmacro()

    # test_main is a .cpp file that contains standard google test main function for running all tests
    package_add_test(test_point_cloud test_matrix3xd_coder.cpp)
    package_add_test(test_extrinsics_encoder test_extrinsics_encoder.cpp)
    package_add_test(test_intrinsics_coder test_intrinsics_coder.cpp)
    package_add_test(test_affine3d_coder test_affine3d_coder.cpp)
    package_add_test(test_no_translation_point_cloud test_no_translation_point_cloud.cpp)
    package_add_test(test_translation_point_cloud test_translation_point_cloud.cpp)

    add_executable(test_main
            test_main.cpp
            test_matrix3xd_coder.cpp
            test_extrinsics_encoder.cpp
            test_intrinsics_coder.cpp
            test_affine3d_coder.cpp
            test_no_translation_point_cloud.cpp
            test_translation_point_cloud.cpp)
    target_link_libraries(test_main Fulfil.DepthCam gtest gmock_main gtest_main pthread)
endif()