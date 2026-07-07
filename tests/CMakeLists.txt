add_executable(f1sim_tests 
    test_car.cpp 
    test_tyre_model.cpp 
    test_monte_carlo.cpp
)

target_link_libraries(f1sim_tests PRIVATE f1sim_core GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(f1sim_tests)