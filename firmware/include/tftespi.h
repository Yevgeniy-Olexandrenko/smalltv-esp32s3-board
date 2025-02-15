#pragma once

#include "board.h"

#define USER_SETUP_LOADED
#define ST7789_2_DRIVER
#define LOAD_GLCD

#define TFT_WIDTH       240
#define TFT_HEIGHT      240
#define TFT_MOSI        PIN_LCD_SDA
#define TFT_SCLK        PIN_LCD_SCL
#define TFT_CS          PIN_LCD_CS
#define TFT_DC          PIN_LCD_DC
#define TFT_RST         PIN_LCD_RES
#define TFT_BL          GPIO_NUM_NC
#define TOUCH_CS        GPIO_NUM_NC
#define SPI_FREQUENCY   80000000
