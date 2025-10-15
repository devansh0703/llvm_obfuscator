cmake_minimum_required(VERSION 3.20)
project(AdaptiveObfuscator VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find LLVM
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# Find required packages
find_package(Threads REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -fPIC")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")

# Source files
set(CORE_SOURCES
    src/core/ObfuscationEngine.cpp
    src/core/InputHandler.cpp
    src/core/PluginLoader.cpp
    src/core/ReportingModule.cpp
)

set(PASS_SOURCES
    src/passes/ControlFlowFlattening.cpp
    src/passes/BogusCodeInjection.cpp
    src/passes/StringObfuscation.cpp
    src/passes/InstructionSubstitution.cpp
    src/passes/OpaquePredicates.cpp
    src/passes/FakeLoopGeneration.cpp
    src/passes/CodeWatermarking.cpp
    src/passes/AntiDebugging.cpp
)

set(AI_SOURCES
    src/ai/AIProfiler.cpp
    src/ai/ProgramAnalyzer.cpp
    src/ai/MLModel.cpp
)

set(JIT_SOURCES
    src/jit/JITEngine.cpp
    src/jit/RuntimeObfuscator.cpp
    src/jit/StubGenerator.cpp
)

set(UTILS_SOURCES
    src/utils/Crypto.cpp
    src/utils/Random.cpp
    src/utils/Metrics.cpp
    src/utils/Logger.cpp
)

# Main library
add_library(ObfuscatorCore SHARED
    ${CORE_SOURCES}
    ${PASS_SOURCES}
    ${AI_SOURCES}
    ${JIT_SOURCES}
    ${UTILS_SOURCES}
)

# Link LLVM libraries
llvm_map_components_to_libnames(llvm_libs
    Core
    Support
    IRReader
    BitWriter
    TransformUtils
    ScalarOpts
    IPO
    Analysis
    Target
    MC
    ExecutionEngine
    OrcJIT
    RuntimeDyld
    X86CodeGen
    X86AsmParser
    X86Desc
    X86Info
)

target_link_libraries(ObfuscatorCore
    ${llvm_libs}
    Threads::Threads
    ${CMAKE_DL_LIBS}
)

# CLI executable
add_executable(obfuscator-cli
    src/cli/main.cpp
)

target_link_libraries(obfuscator-cli ObfuscatorCore)

# Install targets
install(TARGETS obfuscator-cli ObfuscatorCore
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/ DESTINATION include)

# Plugin examples
add_subdirectory(plugins/example)

# Tests
enable_testing()
add_subdirectory(tests)

# Examples
add_subdirectory(examples)
