#pragma once

#include <GyverDB.h>

// hardware
DB_KEYS(storage, type);
DB_KEYS(display, brightness, brightness_n);
DB_KEYS(audio,   volume, volume_n);

// software
DB_KEYS(theme,   color, casing);
DB_KEYS(wifi,    ssid, pass, tout);
DB_KEYS(geo,     method, latitude, longitude, timezone);
DB_KEYS(player,  pl_shuffle, pl_loop);
DB_KEYS(reboot,  to_msc);

// apikeys
DB_KEYS(apikey,  google, openweather);
