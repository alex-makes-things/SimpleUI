#pragma once
#include <random>
#include <string>
#include <stdint.h>

/*This is a very very simple UUID v4 generator, the random numbers are generated in a sequence that doesn't change when rebooted, but it's pretty safe
to say that it's never gonna give the same UUID twice in a single boot.*/

namespace UUIDbuddy{
    inline std::random_device random_device;
    inline std::mt19937 random_engine{random_device()};
    inline std::uniform_int_distribution<int> distribution{0, 255};
    //Returns an std::string containing a Version 4 UUID
    inline std::string generateUUID(){
        uint8_t bytes[16];
        for(uint8_t &byte : bytes){
            byte = static_cast<uint8_t>(distribution(random_engine));
        }
        bytes[6] = (bytes[6] & 0x0F) | 0x40; // Version 4
        bytes[8] = (bytes[8] & 0x3F) | 0x80; // Variant
        char buffer[37];
        std::sprintf(buffer, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            bytes[0], bytes[1], bytes[2], bytes[3],
            bytes[4], bytes[5],
            bytes[6], bytes[7],
            bytes[8], bytes[9],
            bytes[10], bytes[11], bytes[12], bytes[13], bytes[14], bytes[15]);
        return std::string(buffer);
    }
}




