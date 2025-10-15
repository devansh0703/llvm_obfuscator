// Simple example program to obfuscate
#include <stdio.h>
#include <string.h>

int calculate_checksum(const char* data, int length) {
    int checksum = 0;
    for (int i = 0; i < length; i++) {
        checksum += data[i];
        checksum ^= (checksum << 3);
    }
    return checksum;
}

void print_banner() {
    printf("==========================\n");
    printf("  Secure Application v1.0\n");
    printf("==========================\n");
}

int verify_license(const char* key) {
    const char* valid_key = "ABC123XYZ";
    
    if (strcmp(key, valid_key) == 0) {
        return 1;
    }
    
    return 0;
}

int main(int argc, char** argv) {
    print_banner();
    
    if (argc < 2) {
        printf("Usage: %s <license-key>\n", argv[0]);
        return 1;
    }
    
    const char* license = argv[1];
    
    if (verify_license(license)) {
        printf("License valid!\n");
        
        // Perform some critical operation
        int checksum = calculate_checksum(license, strlen(license));
        printf("Checksum: %d\n", checksum);
        
        printf("Access granted.\n");
        return 0;
    } else {
        printf("Invalid license key.\n");
        return 1;
    }
}
