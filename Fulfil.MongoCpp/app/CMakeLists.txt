if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
   # Add the executable code for your application main function
   add_executable(main main.cpp)
   # Link to your library, which also links to all third party libraries used
   target_link_libraries(main PRIVATE FulfilMongoCpp)

endif()
