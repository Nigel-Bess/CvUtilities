if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
      
   # test_main is a .cpp file that contains standard google test main function for running all tests
   # This will create an executable named 'test_main' when built through docker build
   # Simply running `./test_main` will run all the tests defined here
   add_executable(test_main
      test_main.cpp
      fixtures.cpp
      tray_calibration_filename_test.cpp
   )
   target_link_libraries(test_main Fulfil.Dispense Fulfil.DepthCam Fulfil.CPPUtils gtest gmock pthread)

   # Register with CTest
   add_test(NAME dispense_tests COMMAND test_main)
endif()
