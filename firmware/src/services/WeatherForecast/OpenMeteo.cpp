#include "OpenMeteo.h"

namespace service::details
{
    bool OpenMeteo::requestWeather(float latitude, float longitude)
    {
        // TODO

        return false;
    }

    WeatherConditions OpenMeteo::getWeatherConditions() const
    {
        switch(WeatherCode(m_weatherCode))
        {
            case WeatherCode::ClearSky:
                return WeatherConditions::ClearSky;

            case WeatherCode::MailyClear:
                return WeatherConditions::FewClouds;

            case WeatherCode::PartlyCloudy:
                return WeatherConditions::ScatteredClouds;

            case WeatherCode::Overcast:
                return WeatherConditions::BrokenClouds;

            case WeatherCode::Fog:
            case WeatherCode::DepositingRimeFog:
                return WeatherConditions::Mist;

            case WeatherCode::LightDrizzle:
            case WeatherCode::LightFreezingDrizzle:
            case WeatherCode::SlightRain:
            case WeatherCode::LightFreezingRain:
            case WeatherCode::SlightRainShowers:
                return WeatherConditions::Rain;

            case WeatherCode::ModerateDrizzle:
            case WeatherCode::DenseIntensityDrizzle:
            case WeatherCode::DenseIntensityFreezingDrizzle:
            case WeatherCode::ModerateRain:
            case WeatherCode::HeavyIntensityRain:
            case WeatherCode::HeavyIntensityFreezingRain:
            case WeatherCode::ModerateRainShowers:
            case WeatherCode::ViolentRainShowers:
                return WeatherConditions::ShowerRain;

            case WeatherCode::SlightSnowFall:
            case WeatherCode::ModerateSnowFall:
            case WeatherCode::SlightSnowShowers:
                return WeatherConditions::SnowFall;

            case WeatherCode::HeavyIntensitySnowFall:
            case WeatherCode::SnowGrains:
            case WeatherCode::HeavySnowShowers:
                return WeatherConditions::HeavySnowFall;

            case WeatherCode::SlightOrModerateThunderstorm:
                return WeatherConditions::Thunderstorm;

            case WeatherCode::SlightThunderstormWithHail:
            case WeatherCode::HeavyThunderstormWithHail:
                return WeatherConditions::HeavyThunderstorm;
        }
        return WeatherConditions::Unknown;
    }

    // TODO

    // request:
    // https://api.open-meteo.com/v1/forecast?latitude=49.9808&longitude=36.2527&daily=weather_code,temperature_2m_max,temperature_2m_min,sunrise,sunset,apparent_temperature_max,apparent_temperature_min,precipitation_probability_max&current=temperature_2m,relative_humidity_2m,weather_code,wind_speed_10m,wind_direction_10m,surface_pressure,apparent_temperature
    const char* urlTemplate = 
        "https://api.open-meteo.com/v1/forecast"
        "?latitude=%.4f"
        "&longitude=%.4f"
        "&daily=weather_code,temperature_2m_max,temperature_2m_min,sunrise,sunset,apparent_temperature_max,apparent_temperature_min,precipitation_probability_max"
        "&current=temperature_2m,relative_humidity_2m,weather_code,wind_speed_10m,wind_direction_10m,surface_pressure,apparent_temperature";

}

