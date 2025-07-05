#pragma once
#include <stdint.h>
#include <math.h>
#include <Arduino.h>

namespace ArrUtils{
    int getArrSize8(int width, int height, float scale_fac);
    int getArrSize16 (int width, int height, float scale_fac);
}

enum class PixelType{Mono, RGB565};

//A wrapper for supporting multiple data types used in the Image structure
struct ImageData{
    PixelType colorspace;
    union{
        uint8_t* mono;
        uint16_t* rgb565;
    };
    ImageData() : colorspace(PixelType::Mono), mono(nullptr) {}
    ImageData(PixelType type, uint8_t *input) : colorspace(type), mono(input) {}
    ImageData(PixelType type, uint16_t *input) : colorspace(type), rgb565(input) {}
};

//A useful and versatile image wrapper that holds dimensions and a pointer to an array of any supported colorspace
struct Image{
    unsigned int width, height;
    ImageData data;
    bool ownsData = false;

    Image(unsigned int w=0, unsigned int h=0, uint8_t* input=nullptr, bool owner = false) : width(w), height(h), data(PixelType::Mono, input), ownsData(owner){}
    Image(unsigned int w, unsigned int h, uint16_t *input, bool owner = false) : width(w), height(h), data(PixelType::RGB565, input), ownsData(owner) {}
    Image(unsigned int w, unsigned int h, const uint8_t *input, bool owner = false) : width(w), height(h), data(PixelType::Mono, (uint8_t *)input), ownsData(owner) {}
    Image(unsigned int w, unsigned int h, const uint16_t *input, bool owner = false) : width(w), height(h), data(PixelType::RGB565, (uint16_t *)input), ownsData(owner) {}
    ImageData getData(){return data;}

    ~Image(){
        if (ownsData) {
            switch (data.colorspace) {
                case PixelType::Mono:   delete[] data.mono;   break;
                case PixelType::RGB565: delete[] data.rgb565; break;
            }
        }
    }
};


float Fmap(float x, float in_min, float in_max, float out_min, float out_max);
float Flerp(float v0, float v1, float t);
Image scale(Image &input, float scaling_factor);