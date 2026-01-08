add_executable(pre_drop_image_side_dispense_request_tests
    pre_drop_image_side_dispense_request_tests.cpp
)

target_link_libraries(pre_drop_image_side_dispense_request_tests
  PRIVATE
    Fulfil.Dispense
    Fulfil.DepthCam
    Fulfil.CPPUtils
    gtest
    gmock
    pthread
)

add_test(NAME pre_drop_image_side_dispense_request_tests COMMAND pre_drop_image_side_dispense_request_tests)
