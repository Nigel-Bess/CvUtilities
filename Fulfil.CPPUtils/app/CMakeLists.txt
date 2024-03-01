if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
   # Add the executable code for your application main function
   add_executable(main main.cpp)
   # Link to your library, which also links to all third party libraries used
   target_link_libraries(main PRIVATE Fulfil.CPPUtils)

   add_executable(create_socket create_socket.cpp)
   target_link_libraries(create_socket PRIVATE Fulfil.CPPUtils pthread)

   add_executable(simple_client simple_client.cpp)
   target_link_libraries(simple_client PRIVATE Fulfil.CPPUtils)

add_executable(grpc_test grpc_test.cpp )
target_link_libraries(grpc_test PRIVATE Fulfil.CPPUtils ${_REFLECTION} ${_GRPC_GRPCPP} ${_PROTOBUF_LIBPROTOBUF})

endif()