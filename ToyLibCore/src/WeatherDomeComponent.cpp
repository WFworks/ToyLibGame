#include "WeatherDomeComponent.h"
#include "SkyDomeMeshGenerator.h"
#include "VertexArray.h"
#include "Renderer.h"
#include "LightingManager.h"
#include "Actor.h"
#include "Application.h"
#include "Renderer.h"
#include "MathUtil.h"
#include "TimeOfDaySystem.h"
#include <algorithm>
#include <cmath>

WeatherDomeComponent::WeatherDomeComponent(Actor* a)
: SkyDomeComponent(a)
, mTime(0.5f)
, mSunDir(Vector3::UnitY)
, mWeatherType(WeatherType::CLEAR)
{
    mSkyVAO = SkyDomeMeshGenerator::CreateSkyDomeVAO(32, 16, 1.0f);
    mOwnerActor->GetApp()->GetRenderer()->RegisterSkyDome(this);
    mShader = mOwnerActor->GetApp()->GetRenderer()->GetShader("SkyDome");
}

void WeatherDomeComponent::SetTime(float t)
{
    mTime = fmod(t, 1.0f);
}

void WeatherDomeComponent::SetSunDirection(const Vector3& dir)
{
    mSunDir = dir;
}


void WeatherDomeComponent::Draw()
{

    if (!mSkyVAO || !mShader) return;
    

    Matrix4 invView = mOwnerActor->GetApp()->GetRenderer()->GetInvViewMatrix();
    
    Vector3 camPos = invView.GetTranslation() + Vector3(0, 50, 0);
    Matrix4 model = Matrix4::CreateScale(200.0f) * Matrix4::CreateTranslation(camPos);
    Matrix4 view = mOwnerActor->GetApp()->GetRenderer()->GetViewMatrix();
    Matrix4 proj = mOwnerActor->GetApp()->GetRenderer()->GetProjectionMatrix();
    Matrix4 mvp = model * view * proj;


    mShader->SetActive();
    mShader->SetMatrixUniform("uMVP", mvp);
    
    float t = fmod(SDL_GetTicks() / 1000.0f, 60.0f) / 60.0f; // 0〜1で60秒周期
    mShader->SetFloatUniform("uTime", t);
    mShader->SetIntUniform("uWeatherType", static_cast<int>(mWeatherType));
    mShader->SetFloatUniform("uTimeOfDay", fmod(mTime, 1.0f)); // 0.0〜1.0
    

    mShader->SetVectorUniform("uSunDir", mSunDir);
   
    mShader->SetVectorUniform("uRawSkyColor",   mRawSkyColor);
    mShader->SetVectorUniform("uRawCloudColor", mRawCloudColor);
    
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE); // Z書き込みを無効
    mSkyVAO->SetActive();
    glDrawElements(GL_TRIANGLES, mSkyVAO->GetNumIndices(), GL_UNSIGNED_INT, nullptr);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
}

float WeatherDomeComponent::SmoothStep(float edge0, float edge1, float x)
{
    // Clamp x between edge0 and edge1
    float t = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}



void WeatherDomeComponent::Update(float deltaTime)
{
    auto timeSystem = mOwnerActor->GetApp()->GetTimeOfDaySystem();
    
    float hour = timeSystem->GetHourFloat();;
    float t = hour / 24.f;
    mTime = t;

    ApplyTime();
    
}



Vector3 WeatherDomeComponent::GetSkyColor(float time)
{
    Vector3 night(0.01f, 0.02f, 0.05f);
    Vector3 day  (0.7f, 0.8f, 1.0f);
    Vector3 dusk (0.9f, 0.4f, 0.2f);

    // time: 0.0〜1.0 を想定（0:00〜24:00）
    time = fmodf(time, 1.0f);
    if (time < 0.0f) time += 1.0f;

    // 日の出/日の入り（時間単位）
    const float sunriseHour  = 5.f;   // 5:00
    const float sunsetHour   = 18.f;  // 18:00
    const float dawnSpanHour = 1.f;   // 日の出前後1時間をグラデーション
    const float duskSpanHour = 1.f;   // 日の入り前後1時間をグラデーション

    // 0〜1 に変換
    const float sunriseT   = sunriseHour   / 24.f;
    const float sunsetT    = sunsetHour    / 24.f;
    const float dawnSpanT  = dawnSpanHour  / 24.f;
    const float duskSpanT  = duskSpanHour  / 24.f;

    // 区間境界
    const float tNightEnd1    = sunriseT - dawnSpanT;   // 夜 → 朝焼け開始
    const float tDawnMid      = sunriseT;               // 夜→dusk の切れ目
    const float tDawnEnd      = sunriseT + dawnSpanT;   // 朝焼け → 昼
    const float tDuskStart    = sunsetT - duskSpanT;    // 昼 → 夕焼け開始
    const float tDuskMid      = sunsetT;                // 昼→dusk の切れ目
    const float tNightStart2  = sunsetT + duskSpanT;    // 夕焼け → 夜

    // --- 夜（日の入り後〜日の出前） ---
    if (time < tNightEnd1 || time >= tNightStart2)
    {
        return night;
    }
    // --- 朝焼け：夜色 → 夕焼け色 ---
    else if (time < tDawnMid)
    {
        float u = (time - tNightEnd1) / (tDawnMid - tNightEnd1);
        return Vector3::Lerp(night, dusk, u);
    }
    // --- 朝焼け：夕焼け色 → 昼空 ---
    else if (time < tDawnEnd)
    {
        float u = (time - tDawnMid) / (tDawnEnd - tDawnMid);
        return Vector3::Lerp(dusk, day, u);
    }
    // --- 昼 ---
    else if (time < tDuskStart)
    {
        return day;
    }
    // --- 夕焼け：昼空 → 夕焼け空 ---
    else if (time < tDuskMid)
    {
        float u = (time - tDuskStart) / (tDuskMid - tDuskStart);
        return Vector3::Lerp(day, dusk, u);
    }
    // --- 夕焼け：夕焼け空 → 夜空 ---
    else // time < tNightStart2 が保証されている
    {
        float u = (time - tDuskMid) / (tNightStart2 - tDuskMid);
        return Vector3::Lerp(dusk, night, u);
    }
}

Vector3 WeatherDomeComponent::GetCloudColor(float time)
{
    Vector3 dayColor   (1.0f, 1.0f, 1.0f);
    Vector3 duskColor  (0.5f, 0.2f, 0.2f);
    Vector3 nightColor (0.2f, 0.2001f, 0.30015f);

    // time: 0.0〜1.0 を想定（0:00〜24:00）
    time = fmodf(time, 1.0f);
    if (time < 0.0f) time += 1.0f;

    // 日の出/日の入り（時間単位）
    const float sunriseHour  = 5.0f;   // 5:00
    const float sunsetHour   = 18.0f;  // 18:00
    const float dawnSpanHour = 1.0f;   // 日の出前後1時間
    const float duskSpanHour = 1.0f;   // 日の入り前後1時間

    // 0〜1 に変換
    const float sunriseT   = sunriseHour   / 24.0f;
    const float sunsetT    = sunsetHour    / 24.0f;
    const float dawnSpanT  = dawnSpanHour  / 24.0f;
    const float duskSpanT  = duskSpanHour  / 24.0f;

    // 区間境界
    const float tNightEnd1    = sunriseT - dawnSpanT;   // 夜 → 朝焼け開始
    const float tDawnMid      = sunriseT;               // 夜雲→dusk雲
    const float tDawnEnd      = sunriseT + dawnSpanT;   // 朝焼け雲 → 昼雲
    const float tDuskStart    = sunsetT - duskSpanT;    // 昼雲 → 夕焼け雲開始
    const float tDuskMid      = sunsetT;                // 昼雲→夕焼け雲
    const float tNightStart2  = sunsetT + duskSpanT;    // 夕焼け雲 → 夜雲

    // --- 夜雲（日の入り後〜日の出前） ---
    if (time < tNightEnd1 || time >= tNightStart2)
    {
        return nightColor;
    }
    // --- 朝焼け：夜雲 → 夕焼け雲 ---
    else if (time < tDawnMid)
    {
        float u = (time - tNightEnd1) / (tDawnMid - tNightEnd1);
        return Vector3::Lerp(nightColor, duskColor, u);
    }
    // --- 朝焼け：夕焼け雲 → 昼雲 ---
    else if (time < tDawnEnd)
    {
        float u = (time - tDawnMid) / (tDawnEnd - tDawnMid);
        return Vector3::Lerp(duskColor, dayColor, u);
    }
    // --- 昼雲 ---
    else if (time < tDuskStart)
    {
        return dayColor;
    }
    // --- 夕焼け：昼雲 → 夕焼け雲 ---
    else if (time < tDuskMid)
    {
        float u = (time - tDuskStart) / (tDuskMid - tDuskStart);
        return Vector3::Lerp(dayColor, duskColor, u);
    }
    // --- 夕焼け：夕焼け雲 → 夜雲 ---
    else // time < tNightStart2 が保証されている
    {
        float u = (time - tDuskMid) / (tNightStart2 - tDuskMid);
        return Vector3::Lerp(duskColor, nightColor, u);
    }
}

void WeatherDomeComponent::ApplyTime()
{
    float timeOfDay = fmod(mTime, 1.0f);

    // --- 太陽方向 ---
    float angle = Math::TwoPi * (timeOfDay - 0.25f);

    const float elevationScale = 0.9f;     // 軌道が低いなら 1.0〜1.2 くらいに上げる
    const float verticalOffset = -0.1f;    // 全体を少し上に持ち上げたい場合

    mSunDir = Vector3(
        -cosf(angle),                                        // 東→西
        -sinf(angle) * elevationScale + verticalOffset,      // 高さ
        0.4f * sinf(angle)                                   // 南寄せ（0.3〜0.5で好み調整）
    );
    mSunDir.Normalize();
    
    // セット（ディレクショナルライトとシェーダー両方に）
    mLightingManager->SetLightDirection(Vector3(-mSunDir.x, -mSunDir.y, -mSunDir.z), Vector3::Zero);
    
    // --- 空のベース色（シェーダと共通のロジック） ---
    mRawSkyColor   = GetSkyColor(timeOfDay);
    mRawCloudColor = GetCloudColor(timeOfDay);

    // --- 天気減衰 ---
    float weatherDim = 1.0f;
    switch (mWeatherType)
    {
        case WeatherType::CLEAR:  weatherDim = 1.0f; break;
        case WeatherType::CLOUDY: weatherDim = 0.7f; break;
        case WeatherType::RAIN:   weatherDim = 0.5f; break;
        case WeatherType::STORM:  weatherDim = 0.3f; break;
        case WeatherType::SNOW:   weatherDim = 0.6f; break;
    }

    // --- 昼夜の強さ ---
    float dayStrength = SmoothStep(0.15f, 0.25f, timeOfDay) *
                       (1.0f - SmoothStep(0.75f, 0.85f, timeOfDay));
    float nightStrength = 1.0f - dayStrength;
    // 太陽の強度を LightingManager に渡す
    mLightingManager->SetSunIntensity(dayStrength);
    
    // --- ライト色 ---
    Vector3 sunColor  = Vector3(1.0f, 0.95f, 0.8f);
    Vector3 moonColor = Vector3(0.3f, 0.4f, 0.6f);
    Vector3 finalLightColor =
        (sunColor * dayStrength + moonColor * nightStrength) * weatherDim;
    mLightingManager->SetLightDiffuseColor(finalLightColor);

    // --- アンビエント色 ---
    Vector3 dayAmbient   = Vector3(0.7f, 0.7f, 0.7f);
    Vector3 nightAmbient = Vector3(0.3f, 0.3f, 0.4f);
    Vector3 finalAmbient =
        (dayAmbient * dayStrength + nightAmbient * nightStrength) * weatherDim;
    mLightingManager->SetAmbientColor(finalAmbient);

    // --- フォグ色＋密度 ---
    ComputeFogFromSky(timeOfDay);
}

void WeatherDomeComponent::ComputeFogFromSky(float timeOfDay)
{
    // GLSL の baseSky ロジックと揃える
    float weatherFade = (mWeatherType == WeatherType::CLEAR) ? 1.0f : 0.3f;
    Vector3 overcastBase(0.4f, 0.4f, 0.5f);

    Vector3 baseSky = Vector3::Lerp(overcastBase, mRawSkyColor, weatherFade);


    // 地平線寄り（少し暗め）
    float t = 0.1f;
    Vector3 skyHorizon = Vector3::Lerp(baseSky * 0.6f, baseSky, t);

    Vector3 cloudColor = mRawCloudColor;
    float cloudMix = 0.0f;

    switch (mWeatherType)
    {
    case WeatherType::CLEAR:
        cloudMix = 0.2f;
        mFogDensity = 0.002f;
        break;
    case WeatherType::CLOUDY:
        cloudMix = 0.5f;
        mFogDensity = 0.004f;
        skyHorizon = Vector3::Lerp(skyHorizon, Vector3(0.3f,0.3f,0.3f), 0.5f);
        break;
    case WeatherType::RAIN:
        cloudMix = 0.7f;
        mFogDensity = 0.008f;
        skyHorizon *= 0.4f;
        break;
    case WeatherType::STORM:
        cloudMix = 0.9f;
        mFogDensity = 0.015f;
        skyHorizon = Vector3(0.15f, 0.15f, 0.15f);
        cloudColor = Vector3(0.7f, 0.7f, 0.7f);
        break;
    case WeatherType::SNOW:
        cloudMix = 0.7f;
        mFogDensity = 0.010f;
        skyHorizon = Vector3(0.30f, 0.30f, 0.32f);
        cloudColor = Vector3(0.9f, 0.9f, 0.9f);
        break;
    }

    Vector3 fog = Vector3::Lerp(skyHorizon, cloudColor, cloudMix);

    // 夜は少し暗めにする
    float nightFactor = 0.0f;
    if (timeOfDay < 0.25f)
    {
        nightFactor = (0.25f - timeOfDay) / 0.25f;
    }
    else if (timeOfDay > 0.75f)
    {
        nightFactor = (timeOfDay - 0.75f) / 0.25f;
    }

    fog *= (1.0f - 0.6f * nightFactor);

    mFogColor = fog;

    if (mLightingManager)
    {
        mLightingManager->SetFogColor(mFogColor);
    }
}
