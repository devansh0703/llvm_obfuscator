#ifndef UTILS_CRYPTO_H
#define UTILS_CRYPTO_H

#include <vector>
#include <cstdint>
#include <string>

namespace obfuscator {

class Crypto {
public:
    // XOR encryption/decryption
    static std::vector<uint8_t> xorEncrypt(const std::vector<uint8_t>& data, uint32_t key);
    static std::vector<uint8_t> xorDecrypt(const std::vector<uint8_t>& data, uint32_t key);
    
    // Simple stream cipher
    static std::vector<uint8_t> streamEncrypt(const std::vector<uint8_t>& data, uint32_t key);
    static std::vector<uint8_t> streamDecrypt(const std::vector<uint8_t>& data, uint32_t key);
    
    // AES-like substitution box
    static uint8_t sbox(uint8_t byte);
    static uint8_t invSbox(uint8_t byte);
    
    // Hash functions
    static uint32_t hash32(const std::vector<uint8_t>& data);
    static uint64_t hash64(const std::vector<uint8_t>& data);
    
    // Generate random key
    static uint32_t generateKey();
};

} // namespace obfuscator

#endif // UTILS_CRYPTO_H
