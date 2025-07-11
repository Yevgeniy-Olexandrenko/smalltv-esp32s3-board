#pragma once

// PSRAM availability
#if !defined(BOARD_HAS_PSRAM) || BOARD_HAS_PSRAM == 0
#define NO_PSRAM
#endif

// USB MSC availability
#if !defined(ARDUINO_USB_MODE) || ARDUINO_USB_MODE == 0
#define NO_USBMSC
#endif

// Input voltage sense
#ifdef NO_VINSENSE
    #undef  PIN_VIN_SEN
    #undef  VIN_SEN_ADC1
    #undef  VIN_SEN_VOL1
    #undef  VIN_SEN_ADC2
    #undef  VIN_SEN_VOL2
#else
    #ifndef PIN_VIN_SEN
    #define PIN_VIN_SEN 3
    #endif
    #if !defined(VIN_SEN_ADC1) \
     || !defined(VIN_SEN_VOL1) \
     || !defined(VIN_SEN_ADC2) \
     || !defined(VIN_SEN_VOL2)
    // calibrated linear dependence of vol-
    // tage on the value read from the ADC
    #define VIN_SEN_ADC1 2580
    #define VIN_SEN_VOL1 4330
    #define VIN_SEN_ADC2 3660
    #define VIN_SEN_VOL2 5830
    #endif
#endif

// Self reset
#if defined(NO_SELFRES) || ARDUINO_SMALLTV_BOARD_REV == 0
    #undef  PIN_ESP_RES
#else
    #ifndef PIN_ESP_RES
    #define PIN_ESP_RES 46
    #endif
#endif

// Built-in button
#ifdef NO_BUTTON
    #undef  PIN_BUTTON
#else
    #ifndef PIN_BUTTON
    #define PIN_BUTTON 0
    #endif
#endif

// Built-in LED
#ifdef NO_LED
    #undef PIN_LED_CAT
    #undef PIN_LED_DIN
#else
    #if ARDUINO_SMALLTV_BOARD_REV == 0
    // simple LED, shares the same 
    // PIN as the built-in button,
    // connected to PIN via cathode
    #define PIN_LED_CAT 0
    #elif !defined(PIN_LED_DIN)
    // RGB LED, uses PIN compatible 
    // with most popular devboards
    #define PIN_LED_DIN 48
    #endif
#endif

// SD Card
#ifdef NO_SDCARD
    #undef  PIN_SD_CLK
    #undef  PIN_SD_CMD
    #undef  PIN_SD_D0
    #undef  PIN_SD_D1
    #undef  PIN_SD_D2
    #undef  PIN_SD_D3
    #undef  PIN_SD_MISO
    #undef  PIN_SD_MOSI
    #undef  PIN_SD_SCK
    #undef  PIN_SD_CS
#else
#ifndef SDCARD_SPI
    #ifndef PIN_SD_CLK
    #define PIN_SD_CLK 41
    #endif
    #ifndef PIN_SD_CMD 
    #define PIN_SD_CMD 38
    #endif
    #ifndef PIN_SD_D0
    #define PIN_SD_D0  42
    #endif
#ifndef SDCARD_SDIO1
    #ifndef PIN_SD_D1
    #define PIN_SD_D1  21
    #endif
    #ifndef PIN_SD_D2
    #define PIN_SD_D2  40
    #endif
    #ifndef PIN_SD_D3
    #define PIN_SD_D3  39
    #endif
#endif
#else
    #ifndef PIN_SD_MISO
    #define PIN_SD_MISO 42
    #endif
    #ifndef PIN_SD_MOSI
    #define PIN_SD_MOSI 38
    #endif
    #ifndef PIN_SD_SCK
    #define PIN_SD_SCK  41
    #endif
    #ifndef PIN_SD_CS
    #define PIN_SD_CS   39
    #endif
#endif
#endif

// Display
#ifdef NO_VIDEO
    #undef  PIN_LCD_BL
    #undef  PIN_LCD_CS
    #undef  PIN_LCD_DC
    #undef  PIN_LCD_RES
    #undef  PIN_LCD_SDA
    #undef  PIN_LCD_SCL
    #undef  LCD_BL_INV
#else
    #ifndef PIN_LCD_BL
    #define PIN_LCD_BL  14
    #endif
    #ifndef PIN_LCD_CS
    #define PIN_LCD_CS  10
    #endif
    #ifndef PIN_LCD_DC
    #define PIN_LCD_DC  13
    #endif
    #ifndef PIN_LCD_RES
    #define PIN_LCD_RES 9
    #endif
    #ifndef PIN_LCD_SDA
    #define PIN_LCD_SDA 11
    #endif
    #ifndef PIN_LCD_SCL
    #define PIN_LCD_SCL 12
    #endif
    #ifdef  ARDUINO_SMALLTV_BOARD
    #undef  LCD_BL_INV
    #endif
#endif

// Sound
#ifdef NO_AUDIO
    #undef  PIN_SND_RLCLK
    #undef  PIN_SND_BCLK
    #undef  PIN_SND_DIN
    #undef  SND_PRE_AMP
#else
    #ifndef PIN_SND_RLCLK
    #define PIN_SND_RLCLK 17
    #endif
    #ifndef PIN_SND_BCLK
    #define PIN_SND_BCLK  16
    #endif
    #ifndef PIN_SND_DIN
    #define PIN_SND_DIN   15
    #endif
    #ifndef SND_PRE_AMP
    #define SND_PRE_AMP   0.75
    #endif
#endif

// Microphone
#ifdef NO_MICROPHONE
    #undef  PIN_MIC_CLK
    #undef  PIN_MIC_DOUT
#else
    #ifndef PIN_MIC_CLK
    #define PIN_MIC_CLK  18
    #endif
    #ifndef PIN_MIC_DOUT
    #define PIN_MIC_DOUT 8
    #endif
#endif

// Touchpads
#ifdef NO_TOUCH0
    #undef  PIN_TOUCH0
#else
    #ifndef PIN_TOUCH0
    #define PIN_TOUCH0 4
    #endif
#endif
#ifdef NO_TOUCH1
    #undef  PIN_TOUCH1
#else
    #ifndef PIN_TOUCH1
    #define PIN_TOUCH1 5
    #endif
#endif
#ifdef NO_TOUCH2
    #undef  PIN_TOUCH2
#else
    #ifndef PIN_TOUCH2
    #define PIN_TOUCH2 6
    #endif
#endif
#ifdef NO_TOUCH3
    #undef  PIN_TOUCH3
#else
    #ifndef PIN_TOUCH3
    #define PIN_TOUCH3 7
    #endif
#endif

// Expansion
#ifdef NO_EXPANSION
    #undef  PIN_ESP_TX
    #undef  PIN_ESP_RX
    #undef  PIN_I2C_SCL
    #undef  PIN_I2C_SDA
#else
    #ifndef PIN_ESP_TX
    #define PIN_ESP_TX  43
    #endif
    #ifndef PIN_ESP_RX
    #define PIN_ESP_RX  44
    #endif
    #ifndef PIN_I2C_SCL
    #define PIN_I2C_SCL 1
    #endif
    #ifndef PIN_I2C_SDA
    #define PIN_I2C_SDA 2
    #endif
#endif
