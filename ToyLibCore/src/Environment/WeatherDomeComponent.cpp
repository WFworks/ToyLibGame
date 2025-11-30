#include "Environment/WeatherDomeComponent.h"
#include "Environment/SkyDomeMeshGenerator.h"
#include "Asset/Geometry/VertexArray.h"
#include "Engine/Render/Renderer.h"
#include "Engine/Render/LightingManager.h"
#include "Engine/Core/Actor.h"
#include "Engine/Core/Application.h"
#include "Engine/Render/Renderer.h"
#include "Utils/MathUtil.h"
#include "Engine/Runtime/TimeOfDaySystem.h"
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
    Vector3 night(0.02f, 0.03f, 0.08f);
    Vector3 day  (0.55f, 0.75f, 1.00f);   // ちょい落ち着いた昼空
    Vector3 dusk (0.95f, 0.55f, 0.35f);   // オレンジ寄り夕焼け

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

    auto smooth01 = [](float x)
    {
        x = Math::Clamp(x, 0.0f, 1.0f);
        return x * x * (3.0f - 2.0f * x); // smoothstep(0,1,x)
    };

    Vector3 col;

    // --- 夜（日の入り後〜日の出前） ---
    if (time < tNightEnd1 || time >= tNightStart2)
    {
        col = night;
    }
    // --- 朝焼け：夜色 → 夕焼け色 ---
    else if (time < tDawnMid)
    {
        float u = (time - tNightEnd1) / (tDawnMid - tNightEnd1);
        u = smooth01(u);
        col = Vector3::Lerp(night, dusk, u);
    }
    // --- 朝焼け：夕焼け色 → 昼空 ---
    else if (time < tDawnEnd)
    {
        float u = (time - tDawnMid) / (tDawnEnd - tDawnMid);
        u = smooth01(u);
        col = Vector3::Lerp(dusk, day, u);
    }
    // --- 昼 ---
    else if (time < tDuskStart)
    {
        float dayMid = (tDawnEnd + tDuskStart) * 0.5f;
        float w = 1.0f - fabsf(time - dayMid) / (tDuskStart - tDawnEnd);
        w = Math::Clamp(w, 0.0f, 1.0f);
        float bright = 0.9f + 0.1f * smooth01(w); // 正午付近で少し明るく
        col = day * bright;
    }
    // --- 夕焼け：昼空 → 夕焼け空 ---
    else if (time < tDuskMid)
    {
        float u = (time - tDuskStart) / (tDuskMid - tDuskStart);
        u = smooth01(u);
        col = Vector3::Lerp(day, dusk, u);
    }
    // --- 夕焼け：夕焼け空 → 夜空 ---
    else // time < tNightStart2 が保証されている
    {
        float u = (time - tDuskMid) / (tNightStart2 - tDuskMid);
        u = smooth01(u);
        col = Vector3::Lerp(dusk, night, u);
    }

    //========================================
    // 晴れ以外の補正
    //========================================
    if (mWeatherType != WeatherType::CLEAR)
    {
        // 明るめの典型的な曇天色（少し青寄りのグレー）
        Vector3 overcast(0.65f, 0.68f, 0.72f);

        // まず時間帯ベースの空色を、かなり強めに曇天に寄せる
        float overcastStrength = 0.75f; // 思い切って 0.75 くらい
        col = Vector3::Lerp(col, overcast, overcastStrength);

        // さらに彩度をガッツリ落として「空の色味」を抑える
        float g = (col.x + col.y + col.z) / 3.0f;
        Vector3 gray(g, g, g);
        // 0.0:元の色, 1.0:完全グレー
        float desat = 0.6f; // 60% くらい無彩色寄りにする
        col = Vector3::Lerp(col, gray, desat);
    }

    return col;
}
/*
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
*/
Vector3 WeatherDomeComponent::GetCloudColor(float time)
{
    // ========= ここは今までの時間ベースの処理 =========
    Vector3 dayColor   (0.95f, 0.98f, 1.00f);
    Vector3 duskColor  (1.00f, 0.75f, 0.55f);
    Vector3 nightColor (0.16f, 0.18f, 0.26f);

    time = fmodf(time, 1.0f);
    if (time < 0.0f) time += 1.0f;

    const float sunriseHour  = 5.0f;
    const float sunsetHour   = 18.0f;
    const float dawnSpanHour = 1.0f;
    const float duskSpanHour = 1.0f;

    const float sunriseT   = sunriseHour   / 24.0f;
    const float sunsetT    = sunsetHour    / 24.0f;
    const float dawnSpanT  = dawnSpanHour  / 24.0f;
    const float duskSpanT  = duskSpanHour  / 24.0f;

    const float tNightEnd1    = sunriseT - dawnSpanT;
    const float tDawnMid      = sunriseT;
    const float tDawnEnd      = sunriseT + dawnSpanT;
    const float tDuskStart    = sunsetT - duskSpanT;
    const float tDuskMid      = sunsetT;
    const float tNightStart2  = sunsetT + duskSpanT;

    auto smooth01 = [](float x)
    {
        x = Math::Clamp(x, 0.0f, 1.0f);
        return x * x * (3.0f - 2.0f * x);
    };

    Vector3 col;

    if (time < tNightEnd1 || time >= tNightStart2)
    {
        col = nightColor;
    }
    else if (time < tDawnMid)
    {
        float u = (time - tNightEnd1) / (tDawnMid - tNightEnd1);
        u = smooth01(u);
        col = Vector3::Lerp(nightColor, duskColor, u);
    }
    else if (time < tDawnEnd)
    {
        float u = (time - tDawnMid) / (tDawnEnd - tDawnMid);
        u = smooth01(u);
        col = Vector3::Lerp(duskColor, dayColor, u);
    }
    else if (time < tDuskStart)
    {
        col = dayColor;
    }
    else if (time < tDuskMid)
    {
        float u = (time - tDuskStart) / (tDuskMid - tDuskStart);
        u = smooth01(u);
        col = Vector3::Lerp(dayColor, duskColor, u);
    }
    else
    {
        float u = (time - tDuskMid) / (tNightStart2 - tDuskMid);
        u = smooth01(u);
        col = Vector3::Lerp(duskColor, nightColor, u);
    }

    // ========= ここから「夜＋悪天候」の暗さ補正 =========

    // 「どれくらい夜か」を 0〜1 で出す
    float nightFactor = 0.0f;
    if (time < tNightEnd1)
    {
        // 深夜〜夜明け前：time=0〜tNightEnd1 を 1→0 にマップ
        nightFactor = 1.0f - (time / tNightEnd1);
    }
    else if (time >= tNightStart2)
    {
        // 夕焼け後〜深夜：tNightStart2〜1 を 0→1 にマップ
        nightFactor = (time - tNightStart2) / (1.0f - tNightStart2);
    }
    nightFactor = Math::Clamp(nightFactor, 0.0f, 1.0f);

    // 対象の天気：曇り・雨・嵐・雪
    bool badWeather =
        (mWeatherType == WeatherType::CLOUDY) ||
        (mWeatherType == WeatherType::RAIN)   ||
        (mWeatherType == WeatherType::STORM)  ||
        (mWeatherType == WeatherType::SNOW);

    if (badWeather && nightFactor > 0.0f)
    {
        // ベースの「暗い夜雲」のターゲット色（少し青い暗グレー）
        Vector3 darkTarget(0.08f, 0.09f, 0.12f);

        // 天気ごとに「どれくらい暗くするか」の係数を変える
        float darkStrength = 0.4f; // CLOUDY の基準
        switch (mWeatherType)
        {
        case WeatherType::CLOUDY: darkStrength = 0.6f; break; // 曇りはちょい暗め
        case WeatherType::RAIN:   darkStrength = 0.8f; break; // 雨はもう少し暗い
        case WeatherType::STORM:  darkStrength = 0.9f; break; // 嵐はかなり暗く
        case WeatherType::SNOW:   darkStrength = 0.5f; break; // 雪は暗いけど少し明るさ残す
        default: break;
        }

        float w = nightFactor * darkStrength;

        // 色そのものを暗いターゲットに寄せる
        col = Vector3::Lerp(col, darkTarget, w);

        // さらに彩度も少し落とすと「夜の空気感」が出る
        float g = (col.x + col.y + col.z) / 3.0f;
        Vector3 gray(g, g, g);
        float desat = 0.4f * nightFactor; // 夜が深いほど無彩色寄りに
        col = Vector3::Lerp(col, gray, desat);
    }

    return col;
}
/*
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
*/
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
    Vector3 nightAmbient = Vector3(0.25f, 0.2f, 0.3f);
    Vector3 finalAmbient =
        (dayAmbient * dayStrength + nightAmbient * nightStrength) * weatherDim;
    mLightingManager->SetAmbientColor(finalAmbient);

    // --- フォグ色＋密度 ---
    ComputeFogFromSky(timeOfDay);
}
void WeatherDomeComponent::ComputeFogFromSky(float timeOfDay)
{
    // 曇り・雨ベースの少し青みがかったグレー
    Vector3 overcastBase(0.4f, 0.4f, 0.5f);

    // 晴れ：1.0 で mRawSkyColor そのまま
    // 曇り：少しだけ夕焼けを混ぜる
    // 雨・嵐・雪：夕焼けは混ぜない
    float weatherFade = 1.0f;
    switch (mWeatherType)
    {
    case WeatherType::CLEAR:
        weatherFade = 1.0f;
        break;
    case WeatherType::CLOUDY:
        weatherFade = 0.25f;    // 曇りはほんのり夕焼け
        break;
    case WeatherType::RAIN:
    case WeatherType::STORM:
    case WeatherType::SNOW:
        weatherFade = 0.0f;     // 夕焼けは一切混ぜない
        break;
    }

    // 晴れ：mRawSkyColor ベース
    // 曇り：overcastBase 75% + mRawSkyColor 25%
    // 雨/嵐/雪：overcastBase 100%
    Vector3 baseSky = Vector3::Lerp(overcastBase, mRawSkyColor, weatherFade);

    // 地平線寄り（少し暗め）
    float t = 0.1f;
    Vector3 skyHorizon = Vector3::Lerp(baseSky * 0.6f, baseSky, t);

    Vector3 cloudColor = mRawCloudColor;
    float cloudMix = 0.0f;

    switch (mWeatherType)
    {
    case WeatherType::CLEAR:
        cloudMix    = 0.2f;
        mFogDensity = 0.002f;
        break;

    case WeatherType::CLOUDY:
        cloudMix    = 0.5f;
        mFogDensity = 0.004f;
        // 曇りは少しだけ暗くして、夕焼け＋グレーの中間くらい
        skyHorizon = Vector3::Lerp(skyHorizon, Vector3(0.3f, 0.3f, 0.32f), 0.4f);
        break;

    case WeatherType::RAIN:
        cloudMix    = 0.7f;
        mFogDensity = 0.008f;
        // 完全に無彩色のグレー（r=g=b）
        skyHorizon  = Vector3(0.20f, 0.20f, 0.21f);
        cloudColor  = skyHorizon; // fog が完全グレーになるよう揃える
        break;

    case WeatherType::STORM:
        cloudMix    = 0.9f;
        mFogDensity = 0.015f;
        // かなり暗いグレー
        skyHorizon  = Vector3(0.12f, 0.12f, 0.13f);
        cloudColor  = skyHorizon;
        break;

    case WeatherType::SNOW:
        cloudMix    = 0.7f;
        mFogDensity = 0.010f;
        // 明るいグレー（少しだけ白寄りだけど無彩色）
        skyHorizon  = Vector3(0.80f, 0.80f, 0.82f);
        cloudColor  = skyHorizon;
        break;
    }

    // 地平線〜雲の中間
    Vector3 fog = Vector3::Lerp(skyHorizon, cloudColor, cloudMix);

    // 夜は少し暗めにする（元のロジックそのまま）
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
