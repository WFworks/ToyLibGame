#pragma once
#include "VisualComponent.h"

class WeatherOverlayComponent : public VisualComponent
{
public:
    WeatherOverlayComponent(class Actor* owner, int drawOrder = 100, VisualLayer layer = VisualLayer::OverlayScreen);
    void Draw() override;
    
    // 雨エフェクトのセット
    void SetRainAmount(const float amt) { mRainAmount = amt; }
    // 霧エフェクトのセット
    void SetFogAmount(const float amt) { mFogAmount = amt; }
    // 雪エフェクトのセット
    void SetSnowAmout(const float amt) { mSnowAmount = amt; }
    
private:
    // 雨の強さ
    float mRainAmount;
    // 霧の強さ
    float mFogAmount;
    // 雪の強さ
    float mSnowAmount;
    
    float mScreenWidth;
    float mScreenHeight;
};
