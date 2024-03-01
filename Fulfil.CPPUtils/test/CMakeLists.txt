if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
   # Setup a macro to easily add new tests to be run by GoogleTest
   macro(package_add_test TESTNAME)
      add_executable(${TESTNAME} ${ARGN})
      target_link_libraries(${TESTNAME} Fulfil.CPPUtils gtest gmock_main gtest_main pthread)
      add_test(NAME ${TESTNAME} COMMAND ${TESTNAME})
      set_target_properties(${TESTNAME} PROPERTIES FOLDER test)
   endmacro()
      
   # test_main is a .cpp file that contains standard google test main function for running all tests
   add_executable(test_main
           test_main.cpp
           test_point_3d.cpp
           test_file_system_helper.cpp
           test_custom_matrix3d_predicate.cpp
           test_matrix3xd_filter.cpp
           processing_queue/processing_queue_tests.cpp
           test_socket_header_serialization.cpp)

   target_link_libraries(test_main Fulfil.CPPUtils gtest gmock_main gtest_main pthread)

   package_add_test(test_point_3d test_point_3d.cpp)
   package_add_test(test_file_system_helper test_file_system_helper.cpp)
   package_add_test(test_custom_matrix3d_predicate test_custom_matrix3d_predicate.cpp)
   package_add_test(test_matrix3xd_filter test_matrix3xd_filter.cpp)
   package_add_test(test_processing_queue processing_queue/processing_queue_tests.cpp)
   package_add_test(test_socket_header_serialization test_socket_header_serialization.cpp)
endif()