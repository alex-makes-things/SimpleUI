#include "constants.h"

SPIClass spi = SPIClass(VSPI);
Adafruit_ST7735 tft = Adafruit_ST7735(&spi, -1, DC, RST);
GFXcanvas16 canvas = GFXcanvas16(128, 64);
