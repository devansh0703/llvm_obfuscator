#include "utils/Random.h"
#include <chrono>

namespace obfuscator {

std::mt19937& Random::getGenerator() {
    static std::mt19937 generator(
        std::chrono::system_clock::now().time_since_epoch().count());
    return generator;
}

void Random::setSeed(uint32_t seed) {
    getGenerator().seed(seed);
}

int Random::randomInt(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(getGenerator());
}

uint32_t Random::randomUInt32() {
    std::uniform_int_distribution<uint32_t> dist;
    return dist(getGenerator());
}

uint64_t Random::randomUInt64() {
    std::uniform_int_distribution<uint64_t> dist;
    return dist(getGenerator());
}

float Random::randomFloat() {
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(getGenerator());
}

double Random::randomDouble() {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(getGenerator());
}

bool Random::randomBool(float probability) {
    return randomFloat() < probability;
}

float Random::probability() {
    return randomFloat();
}

} // namespace obfuscator
