#pragma once

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
    #define PIN_LCD_BL  5
    #endif
    #ifndef PIN_LCD_CS
    #define PIN_LCD_CS  -1
    #endif
    #ifndef PIN_LCD_DC
    #define PIN_LCD_DC  0
    #endif
    #ifndef PIN_LCD_RES
    #define PIN_LCD_RES 2
    #endif
    #ifndef PIN_LCD_SDA
    #define PIN_LCD_SDA 13
    #endif
    #ifndef PIN_LCD_SCL
    #define PIN_LCD_SCL 14
    #endif
#endif
