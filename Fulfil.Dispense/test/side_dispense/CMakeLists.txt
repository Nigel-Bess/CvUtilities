add_executable(side_dispense_tests
    side_dispense_test.cpp
)

target_link_libraries(side_dispense_tests
  PRIVATE
    Fulfil.Dispense
    Fulfil.DepthCam
    Fulfil.CPPUtils
    gtest
    gmock
    pthread
)

add_test(NAME side_dispense_tests COMMAND side_dispense_tests)
