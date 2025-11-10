add_executable(tray_calibration_tests
  tray_calibration_filename_test.cpp
)

target_link_libraries(tray_calibration_tests
  PRIVATE
    Fulfil.Dispense
    Fulfil.DepthCam
    Fulfil.CPPUtils
    gtest
    gmock
    pthread
)

add_test(NAME tray_calibration_tests COMMAND tray_calibration_tests)