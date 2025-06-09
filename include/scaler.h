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

/*  USAGE EXAMPLE:

    float scaling_factor = 0.25;
    pair<float,float> imsize = {128, 64};
    int arraysize = int(floor((imsize.first*scaling_factor)*(imsize.second*scaling_factor)));
    uint16_t* output = new uint16_t[arraysize];
    //pair<unsigned int, unsigned int> imagesize = scale(nicerlandscape, 128, 64, output, scaling_factor);
    delete[] output;
*/