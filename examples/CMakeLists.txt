cmake_minimum_required(VERSION 3.20)

# Example programs
add_custom_target(examples)

# Make run script executable
add_custom_command(
    TARGET examples POST_BUILD
    COMMAND chmod +x ${CMAKE_CURRENT_SOURCE_DIR}/run_example.sh
)
