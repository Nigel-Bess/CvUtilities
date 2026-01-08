add_executable(drop_zone_searcher_tests
    drop_zone_searcher_tests.cpp
)

target_link_libraries( drop_zone_searcher_tests
  PRIVATE
    Fulfil.Dispense
    Fulfil.DepthCam
    Fulfil.CPPUtils
    gtest
    gmock
    pthread
)

add_test(NAME  drop_zone_searcher_tests COMMAND  drop_zone_searcher_tests)
