if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
   # Setup a macro to easily add new tests to be run by GoogleTest
   macro(package_add_test TESTNAME)
      add_executable(${TESTNAME} ${ARGN} fixtures.cpp)
      target_link_libraries(${TESTNAME} Fulfil.Dispense Fulfil.DepthCam Fulfil.CPPUtils gtest gmock_main gtest_main pthread)
      add_test(NAME ${TESTNAME} COMMAND ${TESTNAME})
      set_target_properties(${TESTNAME} PROPERTIES FOLDER test)
   endmacro()
      
   # test_main is a .cpp file that contains standard google test main function for running all tests
   add_executable(test_main
           test_main.cpp
           realsense_image_file_manager_tests.cpp
           fixtures.cpp
           )
   package_add_test(realsense_image_file_manager_tests realsense_image_file_manager_tests.cpp)
   target_link_libraries(test_main Fulfil.Dispense Fulfil.DepthCam Fulfil.CPPUtils gtest gmock_main gtest_main pthread)
endif()
