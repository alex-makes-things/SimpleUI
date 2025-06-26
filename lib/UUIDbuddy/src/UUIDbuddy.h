#pragma once
#include <random>
#include <string>
#include <stdint.h>

/*This is a very very simple UUID v4 generator, the random numbers are generated in a sequence that doesn't change when rebooted, but it's pretty safe
to say that it's never gonna give the same UUID twice in a single boot.*/

namespace RandomBuddy{
    extern std::random_device random_device;
    extern std::mt19937 random_engine;
    extern std::uniform_int_distribution<int> distribution;
    std::string generateUUID();
}