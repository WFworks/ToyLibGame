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
    
    void SetTimeSpeed(const float speed) { mTimeSpeed = speed; }
    void SetWeatherOverlay(class WeatherOverlayComponent* overlay) { mWeatherOverlay = overlay; }
    void SetWeatherDome(class WeatherDomeComponent* dome) { mWeatherDome = dome; }
private:
    class WeatherOverlayComponent* mWeatherOverlay;
    class WeatherDomeComponent* mWeatherDome;
    
    float mTime;
    float mTimeSpeed;
    
    void RandomizeWeather();
    
    int mCount;
    

};
