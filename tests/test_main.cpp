#include <iostream>
#include <cstring>

// Simple test framework
int g_tests_run = 0;
int g_tests_passed = 0;

#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    std::cout << "Running " << #name << "..."; \
    g_tests_run++; \
    test_##name(); \
    g_tests_passed++; \
    std::cout << " PASSED\n"; \
} while(0)

#define ASSERT(condition) do { \
    if (!(condition)) { \
        std::cerr << "ASSERTION FAILED: " << #condition << "\n"; \
        std::cerr << "  File: " << __FILE__ << ":" << __LINE__ << "\n"; \
        throw std::runtime_error("Test failed"); \
    } \
} while(0)

// Declare test functions
void test_control_flow_flattening();
void test_bogus_code_injection();
void test_string_obfuscation();
void test_ai_profiler();

int main(int argc, char** argv) {
    std::cout << "=================================\n";
    std::cout << "Adaptive Obfuscator - Test Suite\n";
    std::cout << "=================================\n\n";
    
    bool run_ai_tests = false;
    
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--ai") == 0) {
            run_ai_tests = true;
        }
    }
    
    try {
        // Run pass tests
        RUN_TEST(control_flow_flattening);
        RUN_TEST(bogus_code_injection);
        RUN_TEST(string_obfuscation);
        
        // Run AI tests if requested
        if (run_ai_tests) {
            RUN_TEST(ai_profiler);
        }
        
        std::cout << "\n=================================\n";
        std::cout << "Tests passed: " << g_tests_passed << "/" << g_tests_run << "\n";
        std::cout << "=================================\n";
        
        return g_tests_passed == g_tests_run ? 0 : 1;
    } catch (const std::exception& e) {
        std::cerr << "\nTest suite failed: " << e.what() << "\n";
        std::cerr << "Tests passed: " << g_tests_passed << "/" << g_tests_run << "\n";
        return 1;
    }
}
