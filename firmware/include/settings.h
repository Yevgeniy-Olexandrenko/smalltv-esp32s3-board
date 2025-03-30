#pragma once

#include <GyverDB.h>

DB_KEYS(db,
    // hardware
    storage_type,
    lcd_brightness,
    lcd_brightness_n,

    // services
    wifi_ssid,
    wifi_pass,
    wifi_tout,
    audio_volume,
    audio_volume_n,
    audio_player_shuffle,
    audio_player_loop,
    
    // reboot
    reboot_to_msc
);
