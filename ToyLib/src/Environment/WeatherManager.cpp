#include "Environment/WeatherManager.h"
#include "Environment/WeatherDomeComponent.h"
#include "Environment/WeatherOverlayComponent.h"

WeatherManager::WeatherManager()
: mWeather(WeatherType::CLEAR)
{
    
}

void WeatherManager::Update(float deltaTime)
{
    if (!mWeatherDome || !mWeatherOverlay) return;
    
    mWeatherDome->SetWeatherType(mWeather);
    ChangeWeather(mWeather);
}


void WeatherManager::ChangeWeather(const WeatherType weather)
{
    
    
    float rainAmount = 0.0f;
    float fogAmount = 0.0f;
    float snowAmount = 0.0f;
    
    switch (weather)
    {
        case WeatherType::CLEAR:
            rainAmount = 0.0f;
            fogAmount = 0.0f;
            break;
        case WeatherType::CLOUDY:
            rainAmount = 0.0f;
            fogAmount = 0.1f;
            break;
        case WeatherType::RAIN:
            rainAmount = 0.4f;
            fogAmount = 0.3f;
            break;
        case WeatherType::STORM:
            rainAmount = 0.7f;
            fogAmount = 0.4f;
            break;
        case WeatherType::SNOW:
            rainAmount = 0.0f;
            fogAmount = 0.7f;
            snowAmount = 0.8f;
            break;
        default:
            break;
    }
    mWeather = weather;

    mWeatherDome->SetWeatherType(mWeather);
    mWeatherOverlay->SetRainAmount(rainAmount);
    mWeatherOverlay->SetFogAmount(fogAmount);
    mWeatherOverlay->SetSnowAmout(snowAmount);
}
