if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    add_executable(main main.cpp)
    target_link_libraries(main Fulfil.DepthCam)

    #Adding executable to generate mock camera data
    add_executable(mock_generator mock_generator.cpp)
    target_link_libraries(mock_generator Fulfil.DepthCam Fulfil.CPPUtils)

    add_executable(marker_displayer marker_displayer.cpp)
    target_link_libraries(marker_displayer Fulfil.DepthCam Fulfil.CPPUtils)
endif()