#pragma once

#include <GyverDB.h>

DB_KEYS(db,
    // hardware
    storage_type,
    lcd_brightness,
    lcd_brightness_n,

    // services
    color_casing,
    color_theme,
    wifi_ssid,
    wifi_pass,
    wifi_tout,
    geo_method,
    geo_latitude,
    geo_longitude,
    geo_timezone,
    audio_volume,
    audio_volume_n,
    audio_player_shuffle,
    audio_player_loop,
    apikey_google,
    apikey_openweather,
    
    // reboot
    reboot_to_msc
);
