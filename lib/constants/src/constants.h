#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#define SDA 21
#define SCL 22
#define DC 23
#define RST 17
#define SCREENHEIGHT 64
#define SCREENWIDTH 128

extern SPIClass spi;
extern Adafruit_ST7735 tft;
extern GFXcanvas16 canvas;