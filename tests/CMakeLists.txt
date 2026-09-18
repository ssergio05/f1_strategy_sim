add_executable(f1_tests
    test_car.cpp
    test_monte_carlo.cpp
    test_tyre_model.cpp
)

# Link against the core F1 simulation library and GoogleTest
target_link_libraries(f1_tests
    PRIVATE
    f1sim_core
    gtest_main
)

# Register the test executable with CTest
include(GoogleTest)
gtest_discover_tests(f1_tests)