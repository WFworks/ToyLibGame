#include "WeatherManager.h"
#include "WeatherDomeComponent.h"
#include "WeatherOverlayComponent.h"
#include <cstdlib>

WeatherManager::WeatherManager()
: mTimeSpeed(0.0f)
, mTime(0.5f)
, mCount(0)
{
    
}

void WeatherManager::Update(float deltaTime)
{
    if (!mWeatherDome || !mWeatherOverlay) return;
    
    mTime += mTimeSpeed;
    mWeatherDome->SetTime(mTime);
    mCount++;
    
    if (mCount % 5000 == 0)
    {
        RandomizeWeather();
    }
}


void WeatherManager::RandomizeWeather()
{
    WeatherType weatherType = WeatherType::CLEAR;
    float rainAmount = 0.0f;
    float fogAmount = 0.0f;
    float snowAmount = 0.0f;
    
    int randVal = rand() % 100;
    if (randVal < 50)
    {
        weatherType = WeatherType::CLEAR;
        rainAmount = 0.0f;
        fogAmount = 0.0f;
    }
    else if (randVal < 70)
    {
        weatherType = WeatherType::CLOUDY;
        rainAmount = 0.0f;
        fogAmount = 0.1f;
    }
    else if (randVal < 85)
    {
        weatherType = WeatherType::RAIN;
        rainAmount = 0.4f;
        fogAmount = 0.3f;
    }
    else if (randVal < 95)
    {
        weatherType = WeatherType::STORM;
        rainAmount = 0.7f;
        fogAmount = 0.4f;
    }
    else
    {
        weatherType = WeatherType::SNOW;
        rainAmount = 0.0f;
        fogAmount = 0.7f;
        snowAmount = 0.8f;
    }

    mWeatherDome->SetWeatherType(weatherType);
    mWeatherOverlay->SetRainAmount(rainAmount);
    mWeatherOverlay->SetFogAmount(fogAmount);
    mWeatherOverlay->SetSnowAmout(snowAmount);
    
}
