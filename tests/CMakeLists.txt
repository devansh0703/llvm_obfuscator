cmake_minimum_required(VERSION 3.20)

# Test suite
enable_testing()

# Create test runner
add_executable(test_runner
    test_main.cpp
    test_passes.cpp
    test_ai_profiler.cpp
)

target_link_libraries(test_runner ObfuscatorCore)

# Add tests
add_test(NAME PassTests COMMAND test_runner)
add_test(NAME AIProfilerTests COMMAND test_runner --ai)

# Integration tests
add_custom_target(integration_tests
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/run_integration_tests.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
