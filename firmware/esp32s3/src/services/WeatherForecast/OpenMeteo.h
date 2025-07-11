#pragma once

#include "WeatherProvider.h"

// implementation for:
// https://open-meteo.com

namespace service::details
{
    enum class WeatherCode
    {
        // Clear sky
        ClearSky = 0,

        // Mainly clear, partly cloudy, and overcast
        MailyClear = 1,
        PartlyCloudy = 2,
        Overcast = 3,

        // Fog and depositing rime fog
        Fog = 45,
        DepositingRimeFog = 48,

        // Drizzle: Light, moderate, and dense intensity
        LightDrizzle = 51,
        ModerateDrizzle = 53,
        DenseIntensityDrizzle = 55,

        // Freezing Drizzle: Light and dense intensity
        LightFreezingDrizzle = 56,
        DenseIntensityFreezingDrizzle = 57,

        // Rain: Slight, moderate and heavy intensity
        SlightRain = 61,
        ModerateRain = 63,
        HeavyIntensityRain = 65,

        // Freezing Rain: Light and heavy intensity
        LightFreezingRain = 66,
        HeavyIntensityFreezingRain = 67,

        // Snow fall: Slight, moderate, and heavy intensity
        SlightSnowFall = 71,
        ModerateSnowFall = 73,
        HeavyIntensitySnowFall = 75,

        // Snow grains
        SnowGrains = 77,

        // Rain showers: Slight, moderate, and violent
        SlightRainShowers = 80,
        ModerateRainShowers = 81, 
        ViolentRainShowers = 82,

        // Snow showers slight and heavy
        SlightSnowShowers = 85,
        HeavySnowShowers = 86,

        // Thunderstorm: Slight or moderate
        SlightOrModerateThunderstorm = 95,

        // Thunderstorm with slight and heavy hail
        SlightThunderstormWithHail = 96,
        HeavyThunderstormWithHail = 99
    };

    class OpenMeteo : public WeatherProvider
    {
    public:
        bool requestWeather(float latitude, float longitude) override;
        WeatherConditions getWeatherConditions() const override;

    private:
        int   m_weatherCode;         // number
        float m_temperature2m;       // celsius
        float m_apparentTemperature; // celsius
        int   m_relativeHumidity2m;  // percents
        float m_surfacePressure;     // hPa
        float m_windSpeed10m;        // km/h,
        int   m_windDirection10m;    // degrees
    };
}
