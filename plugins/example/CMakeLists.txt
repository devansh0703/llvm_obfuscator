cmake_minimum_required(VERSION 3.20)

# Example plugin
add_library(example_plugin SHARED
    example_plugin.cpp
)

target_include_directories(example_plugin PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(example_plugin
    ${llvm_libs}
)

# Install plugin
install(TARGETS example_plugin
    LIBRARY DESTINATION lib/obfuscator/plugins
)
