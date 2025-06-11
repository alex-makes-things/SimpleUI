#include <iostream>
#include <fstream>
#include <stdint.h>
#include <math.h>
#include <iomanip>
#include <memory>
#include <cassert>
#include <algorithm>
#include <iterator>

using namespace std;

int getArrSize8(int width, int height, float scale_fac) {
    int w = int(floor(width * scale_fac));
    int h = int(floor(height * scale_fac));
    int bytes_per_row = (w + 7) / 8;  // round up to nearest byte
    return bytes_per_row * h;
}

int getArrSize16 (int width, int height, float scale_fac){
    return (floor(width*scale_fac)*floor(height*scale_fac));
}

struct Image8{
    unsigned int width, height;
    size_t dataSize = 0;
    uint8_t* data = nullptr;
    Image8(unsigned int w, unsigned int h) : width(w), height(h) {}
    void setSize(unsigned int s){
        dataSize = s;
        data = new uint8_t[s];
    }
    void setData(const uint8_t* input) {
        assert(data);         // Ensure data is allocated
        assert(dataSize > 0); // Ensure size is valid
        copy(input, input + dataSize, data);
    }
};

struct Image16{
    unsigned int width, height;
    size_t dataSize = 0;
    uint16_t* data = nullptr;
    Image16(unsigned int w, unsigned int h) : width(w), height(h) {}
    void setSize(unsigned int s){
        dataSize = s;
        data = new uint16_t[s];
    }
    void setData(const uint16_t* input) {
        assert(data);         // Ensure data is allocated
        assert(dataSize > 0); // Ensure size is valid
        copy(input, input + dataSize, data);
    }
};

pair<unsigned const int, unsigned const int> scale(const uint16_t* input, int width, int height, uint16_t* result, float scaling_factor){
    unsigned int count = 0;
    unsigned const int newwidth = floor(width*scaling_factor);
    unsigned const int newheight = floor(height*scaling_factor);
    for(unsigned int b=0; b < newheight; b++){
        for(unsigned int i=0; i < newwidth; i++){
            unsigned const int scaled_h = floor(b/scaling_factor);
            unsigned const int scaled_w = floor(i/scaling_factor);
            result[count] = input[(scaled_h * width) + scaled_w];
            count++;
        }
    }
    pair<unsigned const int, unsigned const int> newsize = {newwidth, newheight};
    return newsize;
}

//NOT YET TESTED, SHOULD WORK
Image16 scale(Image16 input, float scaling_factor){
    Image16 output(floor(input.width*scaling_factor), input.height*scaling_factor);
    output.setSize(getArrSize16(output.width, output.height, 1.0f));
    for(unsigned int b=0; b < output.height; b++){
        for(unsigned int i=0; i < output.width; i++){
            unsigned const int scaled_h = floor(b/scaling_factor);
            unsigned const int scaled_w = floor(i/scaling_factor);
            output.data[b * output.width + i] = input.data[(scaled_h * input.width) + scaled_w];
        }
    }
    return output;
}

pair<unsigned const int, unsigned const int> scale(const uint8_t* input, int width, int height, uint8_t* result, float scaling_factor){
    unsigned const int newwidth  = floor(width * scaling_factor);
    unsigned const int newheight = floor(height * scaling_factor);
    const int in_row_bytes  = (width + 7) / 8;
    const int out_row_bytes = (newwidth + 7) / 8;
    // Clear the output buffer
    
    const int out_size = out_row_bytes * newheight;
    for (int i = 0; i < out_size; i++) {
        result[i] = 0;
    }
    for (unsigned int y = 0; y < newheight; y++) {
        for (unsigned int x = 0; x < newwidth; x++) {
            // Map to source pixel using nearest neighbor
            unsigned const int src_y = floor(y / scaling_factor);
            unsigned const int src_x = floor(x / scaling_factor);
            // Get the index of the byte containing the pixel we look for
            const int src_byte_index = src_y * in_row_bytes + src_x / 8;
            const uint8_t src_mask = 0x80 >> (src_x % 8);
            const bool pixel_on = input[src_byte_index] & src_mask;  //See if pixel is on or off
            // Set the output pixel at the scaled coordinates to the result of the previous check
            if (pixel_on) {
                const int dst_byte_index = y * out_row_bytes + x / 8;
                const uint8_t dst_mask = 0x80 >> (x % 8);
                result[dst_byte_index] |= dst_mask;
            }
        }
    }
    pair<unsigned const int, unsigned const int> newsize = {newwidth, newheight};
    return newsize;
}

Image8 scale(Image8 input, float scaling_factor){
    unsigned int scaled_width  = floor(input.width  * scaling_factor);
    unsigned int scaled_height = floor(input.height * scaling_factor);

    Image8 output(scaled_width, scaled_height);

    int outputArrSize = getArrSize8(output.width, output.height, 1.0f);
    output.setSize(outputArrSize);

    const int in_row_bytes  = (input.width + 7) / 8;
    const int out_row_bytes = (output.width + 7) / 8;

    // Clear the output buffer
    /*
    for (int i = 0; i < outputArrSize; i++) {
        output.data[i] = 0;
    }
    */
    fill(output.data, output.data + output.dataSize, 0);

    for (unsigned int y = 0; y < output.height; y++) {
        for (unsigned int x = 0; x < output.width; x++) {
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
                output.data[dst_byte_index] |= dst_mask;
            }
        }
    }
    return output;
}



/*  USAGE EXAMPLE:

    float scaling_factor = 0.25;
    pair<float,float> imsize = {128, 64};
    int arraysize = int(floor((imsize.first*scaling_factor)*(imsize.second*scaling_factor)));
    uint16_t* output = new uint16_t[arraysize];
    //pair<unsigned int, unsigned int> imagesize = scale(nicerlandscape, 128, 64, output, scaling_factor);
    delete[] output;
*/