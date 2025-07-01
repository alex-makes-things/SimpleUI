#pragma once
#include <stdint.h>
#include <math.h>
#include <memory>
#include <algorithm>
#include <functional>


namespace ArrUtils{
    int getArrSize8(int width, int height, float scale_fac);
    int getArrSize16 (int width, int height, float scale_fac);
}

//Stores the pointer to a monochrome image, and its dimensions
struct Image8{
    size_t dataSize = 0;
    unsigned int width = 0;
    unsigned int height = 0;
    uint16_t color = 0xffff;
    uint8_t* data = nullptr;
    Image8(unsigned int w=0, unsigned int h=0) : width(w), height(h) {}

    Image8(unsigned int w, unsigned int h, uint8_t* input, uint16_t hue = 0xffff) : width(w), height(h){
        setSize(ArrUtils::getArrSize8(w,h,1.0f));
        setData(input);
        setColor(hue);
    }

    Image8(unsigned int w, unsigned int h, const uint8_t* input, uint16_t hue = 0xffff) : width(w), height(h){
        setSize(ArrUtils::getArrSize8(w,h,1.0f));
        data = (uint8_t*)input;
        setColor(hue);
    }

    
    /*~Image8(){    This breaks everything
        if(data){
            delete[] data;
        }
    }*/

    void setSize(unsigned int s)
    {
        dataSize = s;
    }
    void setData(uint8_t* input) {
        data = input;
    }
    void setColor(uint16_t hue){
        color = hue;
    }
    void setImg(unsigned int w, unsigned int h, uint8_t* d) {
        width = w;
        height = h;
        data = d;
    }
};

//Stores the pointer to an RGB 16-bit image, along with its dimensions
struct Image16{
    size_t dataSize = 0;
    unsigned int width = 0;
    unsigned int height = 0;
    uint16_t* data = nullptr;
    Image16(unsigned int w=0, unsigned int h=0) : width(w), height(h) {}

    Image16(unsigned int w, unsigned int h, uint16_t* input) : width(w), height(h){
        setSize(ArrUtils::getArrSize8(w,h,1.0f));
        setData(input);
    }

    Image16(unsigned int w, unsigned int h, const uint16_t* input) : width(w), height(h){
        setSize(ArrUtils::getArrSize16(w,h,1.0f));
        data = (uint16_t*)input;
    }

    void setSize(unsigned int s){
        dataSize = s;
    }
    void setData(uint16_t* input) {
        data = input;
    }
    void setImg(unsigned int w, unsigned int h, uint16_t* d) {
        width = w;
        height = h;
        data = d;
    }
};

float mapF(float x, float in_min, float in_max, float out_min, float out_max);
int zlerp(float v0, float v1, float t);
float zlerpF(float v0, float v1, float t);
float zclamp(float n, float min, float max);
std::shared_ptr<Image16> scale(Image16 input, float scaling_factor);
std::shared_ptr<Image8> scale(Image8 input, float scaling_factor);

inline std::function<void(Image16 *)> scaled_image16_deleter = [](Image16 *img)
{
    if (img)
    {
        delete[] img->data; // delete the raw array
        delete img;         // delete the Image8 object itself
    }
};

inline std::function<void(Image8 *)> scaled_image8_deleter = [](Image8 *img)
{
    if (img)
    {
        delete[] img->data; // delete the raw array
        delete img;         // delete the Image8 object itself
    }
};