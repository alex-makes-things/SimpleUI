#include <ImageAssist.h>

float mapF(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
int zlerp(float v0, float v1, float t) {
  return int(round((1 - t) * v0 + t * v1));
}
float zlerpF(float v0, float v1, float t) {
  return (1 - t) * v0 + t * v1;
}
float zclamp(float n, float min, float max) {
  return std::max(min, std::min(n, max));
}



int ArrUtils::getArrSize8(int width, int height, float scale_fac) {
        int w = int(floor(width * scale_fac));
        int h = int(floor(height * scale_fac));
        int bytes_per_row = (w + 7) / 8;  // round up to nearest byte
        return bytes_per_row * h;
    }

int ArrUtils::getArrSize16 (int width, int height, float scale_fac){
        return (floor(width*scale_fac)*floor(height*scale_fac));
    }

//NOT YET TESTED, SHOULD WORK
std::shared_ptr<Image16> scale(Image16 input, float scaling_factor){
    unsigned int scaled_width  = floor(input.width  * scaling_factor);
    unsigned int scaled_height = floor(input.height * scaling_factor);
    size_t outputArrSize = ArrUtils::getArrSize16(scaled_width, scaled_height, 1.0f);
    uint16_t* buffer = new uint16_t[outputArrSize];
    
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

    size_t outputArrSize = ArrUtils::getArrSize8(scaled_width, scaled_height, 1.0f);
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
    return std::shared_ptr<Image8>(new Image8(scaled_width, scaled_height, buffer, input.color), scaled_image8_deleter);
}