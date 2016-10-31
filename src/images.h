#include <Arduino.h>
#define WiFi_Logo_width 60
#define WiFi_Logo_height 36

const char WiFi_Logo_bits[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xFF, 0x07, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xE0, 0xFF, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0xFF,
  0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0xFF, 0xFF, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xFE, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
  0xFF, 0x03, 0x00, 0x00, 0x00, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
  0x00, 0xFF, 0xFF, 0xFF, 0x07, 0xC0, 0x83, 0x01, 0x80, 0xFF, 0xFF, 0xFF,
  0x01, 0x00, 0x07, 0x00, 0xC0, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x0C, 0x00,
  0xC0, 0xFF, 0xFF, 0x7C, 0x00, 0x60, 0x0C, 0x00, 0xC0, 0x31, 0x46, 0x7C,
  0xFC, 0x77, 0x08, 0x00, 0xE0, 0x23, 0xC6, 0x3C, 0xFC, 0x67, 0x18, 0x00,
  0xE0, 0x23, 0xE4, 0x3F, 0x1C, 0x00, 0x18, 0x00, 0xE0, 0x23, 0x60, 0x3C,
  0x1C, 0x70, 0x18, 0x00, 0xE0, 0x03, 0x60, 0x3C, 0x1C, 0x70, 0x18, 0x00,
  0xE0, 0x07, 0x60, 0x3C, 0xFC, 0x73, 0x18, 0x00, 0xE0, 0x87, 0x70, 0x3C,
  0xFC, 0x73, 0x18, 0x00, 0xE0, 0x87, 0x70, 0x3C, 0x1C, 0x70, 0x18, 0x00,
  0xE0, 0x87, 0x70, 0x3C, 0x1C, 0x70, 0x18, 0x00, 0xE0, 0x8F, 0x71, 0x3C,
  0x1C, 0x70, 0x18, 0x00, 0xC0, 0xFF, 0xFF, 0x3F, 0x00, 0x00, 0x08, 0x00,
  0xC0, 0xFF, 0xFF, 0x1F, 0x00, 0x00, 0x0C, 0x00, 0x80, 0xFF, 0xFF, 0x1F,
  0x00, 0x00, 0x06, 0x00, 0x80, 0xFF, 0xFF, 0x0F, 0x00, 0x00, 0x07, 0x00,
  0x00, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0xF8, 0xFF, 0xFF,
  0xFF, 0x7F, 0x00, 0x00, 0x00, 0x00, 0xFE, 0xFF, 0xFF, 0x01, 0x00, 0x00,
  0x00, 0x00, 0xFC, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0xFF,
  0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0xFF, 0x1F, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x80, 0xFF, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  };

const char activeSymbol[] PROGMEM = {
    B00000000,
    B00000000,
    B00011000,
    B00100100,
    B01000010,
    B01000010,
    B00100100,
    B00011000
};

const char inactiveSymbol[] PROGMEM = {
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00011000,
    B00011000,
    B00000000,
    B00000000
};

#define APP_LOGO_WIDTH 40
#define APP_LOGO_HEIGHT 40
const char App_Logo_Image_bits[] PROGMEM = {
    0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x00,
    0x00, 0x7E, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x02, 0x00, 0x7E,
    0x00, 0x80, 0x07, 0x00, 0x3E, 0x00, 0xC0, 0x07, 0x00, 0x7E, 0x00, 0xC0,
    0x0F, 0x00, 0x3E, 0x00, 0xC0, 0x0F, 0x00, 0x7E, 0x00, 0xC0, 0x07, 0x00,
    0x3E, 0x00, 0xC0, 0xEF, 0xF1, 0x7E, 0x00, 0x80, 0xE7, 0x7B, 0x3E, 0x00,
    0xC0, 0xF7, 0x7B, 0x7D, 0x00, 0xC0, 0xEF, 0xBB, 0x39, 0x00, 0xC0, 0xE7,
    0xBB, 0x77, 0x00, 0xC0, 0xFF, 0xBB, 0x67, 0x00, 0xC0, 0xEF, 0xBB, 0x1F,
    0x00, 0x80, 0xF7, 0xBB, 0x3F, 0x00, 0xC0, 0xEF, 0x7B, 0x7F, 0x00, 0xC0,
    0xCF, 0xF9, 0xFE, 0x00, 0xC0, 0x1F, 0xFA, 0xFC, 0x01, 0xC0, 0xFF, 0x73,
    0xFA, 0x01, 0xC0, 0x07, 0x00, 0xF0, 0x03, 0x80, 0xFF, 0x7F, 0xEE, 0x03,
    0x80, 0xFF, 0x3F, 0xDF, 0x01, 0x80, 0xFF, 0xCF, 0xFF, 0x01, 0x80, 0xFF,
    0xFF, 0xFF, 0x01, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF,
    0x00, 0x00, 0xFE, 0xFF, 0x7F, 0x00, 0x00, 0xFC, 0xFF, 0x3F, 0x00, 0x00,
    0xF8, 0xFF, 0x1F, 0x00, 0x00, 0xF0, 0xFF, 0x0F, 0x00, 0x00, 0xC0, 0xFF,
    0x03, 0x00, 0x00, 0x18, 0xFE, 0x1C, 0x00, 0x00, 0xF8, 0x01, 0x1F, 0x00,
    0x00, 0xF0, 0xFF, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8,
    0xFF, 0x1F, 0x00, 0x00, 0xF8, 0xFF, 0x1F, 0x00,  };
