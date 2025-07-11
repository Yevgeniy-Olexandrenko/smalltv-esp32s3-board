#pragma once

#include "board.h"

#define DISABLE_ALL_LIBRARY_WARNINGS
#ifndef NO_VIDEO

// defines
#define USER_SETUP_LOADED
#define ST7789_2_DRIVER
#define LOAD_GLCD

// defined values
#define TFT_RGB_ORDER TFT_RGB
#define TFT_WIDTH     240
#define TFT_HEIGHT    240

// mcu specific
#define USE_FSPI_PORT
#define SPI_FREQUENCY 80000000

// defaults
#undef TFT_BL
#undef TFT_CS
#undef TOUCH_CS

// required pins
#define TFT_DC    PIN_LCD_DC
#define TFT_RST   PIN_LCD_RES
#define TFT_MOSI  PIN_LCD_SDA
#define TFT_SCLK  PIN_LCD_SCL

// optional pins
#if PIN_LCD_CS != -1
#define TFT_CS    PIN_LCD_CS
#endif

#endif
