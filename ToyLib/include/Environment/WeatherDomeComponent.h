#pragma once
#include "Environment/SkyDomeComponent.h"
#include "Environment/WeatherManager.h"


class WeatherDomeComponent : public SkyDomeComponent
{
public:
    WeatherDomeComponent(class Actor* a);
    
    void Draw() override;
    void Update(float deltaTime) override;
    
    void SetTime(float t);
    void SetSunDirection(const Vector3& dir);

    WeatherType GetWeatherType() const { return mWeatherType; }
    void SetWeatherType(WeatherType weather) { mWeatherType = weather; }
    
private:
    float mTime;
    Vector3 mSunDir;
    WeatherType mWeatherType;

    Vector3 mRawSkyColor;
    Vector3 mRawCloudColor;
    Vector3 mFogColor;
    float   mFogDensity;
    
    float SmoothStep(float edge0, float edge1, float x);
    void ApplyTime();
    
    void ComputeFogFromSky(float timeOfDay);
    Vector3 GetCloudColor(float time);
    Vector3 GetSkyColor(float time);
};
