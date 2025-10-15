#ifndef UTILS_RANDOM_H
#define UTILS_RANDOM_H

#include <cstdint>
#include <random>
#include <algorithm>
#include <vector>

namespace obfuscator {

class Random {
public:
    // Initialize with seed
    static void setSeed(uint32_t seed);
    
    // Generate random integer in range [min, max]
    static int randomInt(int min, int max);
    
    // Generate random unsigned integer
    static uint32_t randomUInt32();
    static uint64_t randomUInt64();
    
    // Generate random float in range [0.0, 1.0]
    static float randomFloat();
    static double randomDouble();
    
    // Generate random boolean with given probability
    static bool randomBool(float probability = 0.5f);
    
    // Get probability value [0.0, 1.0]
    static float probability();
    
    // Shuffle vector
    template<typename T>
    static void shuffle(std::vector<T>& vec);
    
private:
    static std::mt19937& getGenerator();
};

template<typename T>
void Random::shuffle(std::vector<T>& vec) {
    std::shuffle(vec.begin(), vec.end(), getGenerator());
}

} // namespace obfuscator

#endif // UTILS_RANDOM_H
