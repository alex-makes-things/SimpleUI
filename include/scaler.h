#include <iostream>
#include <fstream>
#include <stdint.h>
#include <math.h>
#include <iomanip>

using namespace std;



pair<unsigned int, unsigned int> scale(const uint16_t* input, int width, int height, uint16_t* result, float scaling_factor){
    
    unsigned int count = 0;
    unsigned int newwidth = floor(width*scaling_factor);
    unsigned int newheight = floor(height*scaling_factor);

    for(unsigned int b=0; b < newheight; b++){
        for(unsigned int i=0; i < newwidth; i++){
            unsigned int scaled_h = floor(b/scaling_factor);
            unsigned int scaled_w = floor(i/scaling_factor);
            result[count] = input[(scaled_h * width) + scaled_w];
            count++;
        }
    }

    pair<unsigned int, unsigned int> newsize = {newwidth, newheight};
    return newsize;
}

pair<unsigned int, unsigned int> scale(const uint8_t* input, int width, int height, uint8_t* result, float scaling_factor){
    
    unsigned int newwidth  = floor(width * scaling_factor);
    unsigned int newheight = floor(height * scaling_factor);

    int in_row_bytes  = (width + 7) / 8;
    int out_row_bytes = (newwidth + 7) / 8;

    // Clear the output buffer
    int out_size = out_row_bytes * newheight;
    for (int i = 0; i < out_size; i++) {
        result[i] = 0;
    }

    for (unsigned int y = 0; y < newheight; y++) {
        for (unsigned int x = 0; x < newwidth; x++) {
            // Map to source pixel using nearest neighbor
            unsigned int src_y = floor(y / scaling_factor);
            unsigned int src_x = floor(x / scaling_factor);

            int src_byte_index = src_y * in_row_bytes + src_x / 8;
            uint8_t src_mask = 0x80 >> (src_x % 8);
            bool pixel_on = input[src_byte_index] & src_mask;

            if (pixel_on) {
                int dst_byte_index = y * out_row_bytes + x / 8;
                uint8_t dst_mask = 0x80 >> (x % 8);
                result[dst_byte_index] |= dst_mask;
            }
        }
    }
    pair<unsigned int, unsigned int> newsize = {newwidth, newheight};
    return newsize;
}

int getArrSize8 (int width, int height, float scale_fac){
    return ((floor((width*scale_fac)+7)/8)*floor(height*scale_fac));
}

int getArrSize16 (int width, int height, float scale_fac){
    return (floor(width*scale_fac)*floor(height*scale_fac));
}

/*  USAGE EXAMPLE:

    float scaling_factor = 0.25;
    pair<float,float> imsize = {128, 64};
    int arraysize = int(floor((imsize.first*scaling_factor)*(imsize.second*scaling_factor)));
    uint16_t* output = new uint16_t[arraysize];
    //pair<unsigned int, unsigned int> imagesize = scale(nicerlandscape, 128, 64, output, scaling_factor);
    delete[] output;
*/