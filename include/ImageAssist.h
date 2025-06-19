#pragma once
#include <stdint.h>
#include <math.h>
#include <iomanip>
#include <memory>
#include <cassert>
#include <algorithm>
#include <iterator>


namespace ArrUtils{
    int getArrSize8(int width, int height, float scale_fac) {
        int w = int(floor(width * scale_fac));
        int h = int(floor(height * scale_fac));
        int bytes_per_row = (w + 7) / 8;  // round up to nearest byte
        return bytes_per_row * h;
    }

    int getArrSize16 (int width, int height, float scale_fac){
        return (floor(width*scale_fac)*floor(height*scale_fac));
    }
}

//Stores the pointer to a monochrome image, and its dimensions
struct Image8{
    size_t dataSize = 0;
    unsigned int width = 0;
    unsigned int height = 0;
    uint8_t* data = nullptr;
    Image8(unsigned int w=0, unsigned int h=0) : width(w), height(h) {}

    Image8(unsigned int w, unsigned int h, uint8_t* input) : width(w), height(h){
        setSize(ArrUtils::getArrSize8(w,h,1.0f));
        setData(input);
    }

    Image8(unsigned int w, unsigned int h, const uint8_t* input) : width(w), height(h){
        setSize(ArrUtils::getArrSize8(w,h,1.0f));
        data = (uint8_t*)input;
    }

    void setSize(unsigned int s){
        dataSize = s;
    }
    void setData(uint8_t* input) {
        data = input;
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
int lerp(float v0, float v1, float t);
float lerpF(float v0, float v1, float t);
float clamp(float n, float min, float max);
std::shared_ptr<Image16> scale(Image16 input, float scaling_factor);
std::shared_ptr<Image8> scale(Image8 input, float scaling_factor);

auto scaled_image16_deleter = [](Image16* img) {
    if (img) {
        delete[] img->data;  // delete the raw array
        delete img;          // delete the Image8 object itself
    }
};
auto scaled_image8_deleter = [](Image8* img) {
    if (img) {
        delete[] img->data;  // delete the raw array
        delete img;          // delete the Image8 object itself
    }
};

//NOT YET TESTED, SHOULD WORK
std::shared_ptr<Image16> scale(Image16 input, float scaling_factor){
    unsigned int scaled_width  = floor(input.width  * scaling_factor);
    unsigned int scaled_height = floor(input.height * scaling_factor);
    uint16_t* buffer = new uint16_t[ArrUtils::getArrSize16(scaled_width, scaled_height, 1.0f)];
    
    for(unsigned int b=0; b < scaled_height; b++){
        for(unsigned int i=0; i < scaled_width; i++){
            unsigned const int scaled_h = floor(b/scaling_factor);
            unsigned const int scaled_w = floor(i/scaling_factor);
            buffer[b * scaled_width + i] = input.data[(scaled_h * input.width) + scaled_w];
        }
    }
    return std::shared_ptr<Image16>(new Image16(scaled_width, scaled_height, buffer), scaled_image16_deleter);
}


std::shared_ptr<Image8> scale(Image8 input, float scaling_factor){
    unsigned int scaled_width  = floor(input.width  * scaling_factor);
    unsigned int scaled_height = floor(input.height * scaling_factor);

    int outputArrSize = ArrUtils::getArrSize8(scaled_width, scaled_height, 1.0f);
    uint8_t* buffer = new uint8_t[outputArrSize];

    const int in_row_bytes  = (input.width + 7) / 8;
    const int out_row_bytes = (scaled_width + 7) / 8;

    std::fill(buffer, buffer + outputArrSize, 0);

    for (unsigned int y = 0; y < scaled_height; y++) {
        for (unsigned int x = 0; x < scaled_width; x++) {
            // Map to source pixel using nearest neighbor
            unsigned const int src_y = floor(y / scaling_factor);
            unsigned const int src_x = floor(x / scaling_factor);
            // Get the index of the byte containing the pixel we look for
            const size_t src_byte_index = src_y * in_row_bytes + src_x / 8;
            const uint8_t src_mask = 0x80 >> (src_x % 8);
            const bool pixel_on = input.data[src_byte_index] & src_mask;  //See if pixel is on or off
            // Set the output pixel at the scaled coordinates to the result of the previous check
            if (pixel_on) {
                const size_t dst_byte_index = y * out_row_bytes + x / 8;
                const uint8_t dst_mask = 0x80 >> (x % 8);
                buffer[dst_byte_index] |= dst_mask;
            }
        }
    }
    return std::shared_ptr<Image8>(new Image8(scaled_width, scaled_height, buffer), scaled_image8_deleter);
}


float mapF(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
int lerp(float v0, float v1, float t) {
  return int(round((1 - t) * v0 + t * v1));
}
float lerpF(float v0, float v1, float t) {
  return (1 - t) * v0 + t * v1;
}
float clamp(float n, float min, float max) {
  return std::max(min, std::min(n, max));
}