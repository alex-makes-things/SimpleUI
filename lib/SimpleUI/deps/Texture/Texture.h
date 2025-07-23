#pragma once
#include <stdint.h>
#include <math.h>
#include <Arduino.h>
#include <string.h>


enum class PixelType{Mono=1, RGB565=16};

//A wrapper for supporting multiple data types used in the Texture structure
struct TextureData{
    PixelType colorspace;
    union{
        uint8_t* mono;
        uint16_t* rgb565;
    };
    TextureData() : colorspace(PixelType::Mono), mono(nullptr) {}
    TextureData(PixelType type, uint8_t *input) : colorspace(type), mono(input) {}
    TextureData(PixelType type, uint16_t *input) : colorspace(type), rgb565(input) {}
};

//A useful and versatile image wrapper that holds dimensions and a pointer to an array of any supported colorspace
struct Texture{
    unsigned int width, height;
    TextureData data;

    Texture(unsigned int w=0, unsigned int h=0, uint8_t* input=nullptr, bool owner = false) : width(w), height(h), data(PixelType::Mono, input), ownsData(owner){}
    Texture(unsigned int w, unsigned int h, uint16_t *input, bool owner = false) : width(w), height(h), data(PixelType::RGB565, input), ownsData(owner) {}
    Texture(unsigned int w, unsigned int h, const uint8_t *input, bool owner = false) : width(w), height(h), data(PixelType::Mono, (uint8_t *)input), ownsData(owner) {}
    Texture(unsigned int w, unsigned int h, const uint16_t *input, bool owner = false) : width(w), height(h), data(PixelType::RGB565, (uint16_t *)input), ownsData(owner) {}
    TextureData getData(){return data;}
    static int getArrSize8(int width, int height, float scale_fac);
    static int getArrSize16 (int width, int height, float scale_fac);

    ~Texture(){
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
bool dirtyRects(Texture first, Texture second);
const float Fmap(const float x, const float in_min, const float in_max, const float out_min, const float out_max);
const float Flerp(const float v0, const float v1, const float t);
const Texture scale(Texture &input, const float scaling_factor);
const uint16_t rgb565(unsigned int r, unsigned int g, unsigned int b);
const uint16_t hex(std::string hex);