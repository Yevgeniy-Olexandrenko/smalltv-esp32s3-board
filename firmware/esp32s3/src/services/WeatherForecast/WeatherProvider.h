#pragma once

namespace service::details
{
    enum class WeatherConditions
    {
        Unknown,           // ?
        ClearSky,          // ☀️
        FewClouds,         // 🌤️
        ScatteredClouds,   // ⛅
        BrokenClouds,      // ☁️
        Rain,              // 🌦
        ShowerRain,        // 🌧️
        Thunderstorm,      // 🌩️
        HeavyThunderstorm, // ⛈️
        SnowFall,          // ❄️
        HeavySnowFall,     // 🌨️
        Mist               // 🌫️
    };

    enum class TemperatureUnits { Celsius, Fahrenheit };
    enum class WindSpeedUnits   { kmh, ms, mph };

    class WeatherProvider
    {
    public:
        virtual bool requestWeather(float latitude, float longitude) = 0;
        virtual WeatherConditions getWeatherConditions() const = 0;

    };
}
