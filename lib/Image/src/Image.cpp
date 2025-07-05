#include <Image.h>

float Fmap(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
float Flerp(float v0, float v1, float t) {
  return (1 - t) * v0 + t * v1;
}
bool dirtyRects(Image first, Image second){
    if(!(first.width == second.width && first.height == second.height && first.data.colorspace == second.data.colorspace)) return false;
    bool areDifferent=true;
    size_t len = first.width * first.height;
    switch(first.data.colorspace){
        case PixelType::Mono:
        for(size_t i = 0; i<len; i++){
            if(first.data.mono[i] != second.data.mono[i]){
                areDifferent=false;
                first.data.mono[i] = second.data.mono[i];
            }
        }
        break;

        case PixelType::RGB565:
        for(size_t i = 0; i<len; i++){
            if(first.data.mono[i] != second.data.mono[i]){
                areDifferent=false;
                first.data.mono[i] = second.data.mono[i];
            }
        }
    }
    return areDifferent;
}
void transferFrame(uint16_t* emitter, uint16_t* receiver, size_t len){
    for(size_t i=0; i < len; i++){
        receiver[i] = emitter[i];
    }
}

int ArrUtils::getArrSize8(int width, int height, float scale_fac) {
        int w = static_cast<int>(width * scale_fac);
        int h = static_cast<int>(height * scale_fac);
        int bytes_per_row = (w + 7) / 8;  // round up to nearest byte
        return bytes_per_row * h;
    }
int ArrUtils::getArrSize16 (int width, int height, float scale_fac){
    return (static_cast<int>(width * scale_fac) * static_cast<int>(height * scale_fac));
    }

Image scale(Image& input, float scaling_factor){
    const unsigned int scaled_width = static_cast<const unsigned int>(input.width * scaling_factor);
    const unsigned int scaled_height = static_cast<const unsigned int>(input.height * scaling_factor);

    if(input.data.colorspace==PixelType::Mono){
        const size_t outputArrSize = ArrUtils::getArrSize8(scaled_width, scaled_height, 1.0f);
        uint8_t *buffer = new uint8_t[outputArrSize];
        std::fill(buffer, buffer + outputArrSize, 0);

        const int in_row_bytes = (input.width + 7) / 8;
        const int out_row_bytes = (scaled_width + 7) / 8;
        const float inv_scaling = 1.0f / scaling_factor;

        for (unsigned int y = 0; y < scaled_height; y++)
        {
            const size_t dst_row_base = y * out_row_bytes;
            unsigned const int src_y = static_cast<unsigned int>(y * inv_scaling);

            for (unsigned int x = 0; x < scaled_width; x++){
                // Map to source pixel using nearest neighbor
                unsigned const int src_x = static_cast<unsigned int>(x * inv_scaling);
                // Get the index of the byte containing the pixel we look for
                const size_t src_byte_index = src_y * in_row_bytes + src_x / 8;
                const uint8_t src_mask = 0x80 >> (src_x % 8);
                const bool pixel_on = input.data.mono[src_byte_index] & src_mask; // See if pixel is on or off

                // Set the output pixel at the scaled coordinates to the result of the previous check
                if (pixel_on){
                    const size_t dst_byte_index = dst_row_base + x / 8;
                    const uint8_t dst_mask = 0x80 >> (x % 8);
                    buffer[dst_byte_index] |= dst_mask;
                }
            }
        }
        return Image(scaled_width, scaled_height, std::move(buffer), true);
    }
    else{
        size_t outputArrSize = ArrUtils::getArrSize16(scaled_width, scaled_height, 1.0f);
        uint16_t *buffer = new uint16_t[outputArrSize];

        const float inv_scaling = 1.0f / scaling_factor;
        for (unsigned int y = 0; y < scaled_height; y++){
            unsigned const int src_y = static_cast<unsigned int>(y * inv_scaling);

            for (unsigned int x = 0; x < scaled_width; x++){
                unsigned const int src_x = static_cast<unsigned int>(x * inv_scaling);
                buffer[y * scaled_width + x] = input.data.rgb565[(src_y * input.width) + src_x];
            }
        }
        return Image(scaled_width, scaled_height, std::move(buffer), true);
    }
}