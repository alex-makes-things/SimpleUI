#pragma once
#include <stdint.h>
#include <math.h>
#include <Arduino.h>
#include <string.h>

namespace ArrUtils{
    int getArrSize8(int width, int height, float scale_fac);
    int getArrSize16 (int width, int height, float scale_fac);
}

enum class PixelType{Mono=1, RGB565=16};

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
    private:
    bool ownsData = false;
};

void transferFrame(uint16_t* emitter, uint16_t* receiver, size_t len);
bool dirtyRects(Image first, Image second);
const float Fmap(const float x, const float in_min, const float in_max, const float out_min, const float out_max);
const float Flerp(const float v0, const float v1, const float t);
const Image scale(Image &input, const float scaling_factor);
const uint16_t rgb565(unsigned int r, unsigned int g, unsigned int b);
const uint16_t hex(std::string hex);