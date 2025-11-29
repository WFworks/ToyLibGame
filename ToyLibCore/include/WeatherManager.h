#pragma once
#include <memory>

enum class WeatherType
{
    CLEAR = 0,
    CLOUDY,
    RAIN,
    STORM,
    SNOW
};

struct WeatherState
{
    WeatherType type;
    float rainAmount;
    float fogAmount;
    float snowAmount;
};

class WeatherManager
{
public:
    WeatherManager();
    void Update(float deltaTime);
    
    void SetWeatherOverlay(class WeatherOverlayComponent* overlay) { mWeatherOverlay = overlay; }
    void SetWeatherDome(class WeatherDomeComponent* dome) { mWeatherDome = dome; }
    
    void ChangeWeather(const WeatherType weather);
    
private:
    class WeatherOverlayComponent* mWeatherOverlay;
    class WeatherDomeComponent* mWeatherDome;
    
    WeatherType mWeather;
    

};
