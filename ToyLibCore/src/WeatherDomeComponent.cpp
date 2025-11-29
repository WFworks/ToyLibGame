#include "WeatherDomeComponent.h"
#include "SkyDomeMeshGenerator.h"
#include "VertexArray.h"
#include "Renderer.h"
#include "LightingManager.h"
#include "Actor.h"
#include "Application.h"
#include "Renderer.h"
#include "MathUtil.h"
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
    
    Vector3 camPos = invView.GetTranslation() + Vector3(0, -30, 0);
    Matrix4 model = Matrix4::CreateScale(100.0f) * Matrix4::CreateTranslation(camPos);
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
    ApplyTime();
}

/*
void WeatherDomeComponent::ApplyTime()
{
    // ゲーム時間 0.0〜1.0 → 0〜180度（π）を回す
    float angle = Math::Pi * fmod(mTime, 1.0f); // 0.0〜π（180°）

    // 軌道の定義：XZ平面で +X からスタート、+Z に向かって傾く円弧
    // 例えば、XY平面ではなく XZ平面に投影しながら、Yも上下に
    mSunDir = Vector3(
        -cosf(angle),        // +X方向から -X方向へ移動
        -sinf(angle),       // 太陽が昇って沈む（Y軸）
        0.5f * sin(angle)  // 南方向に傾ける（+Z成分）
    );
    mSunDir.Normalize();

    // セット（ディレクショナルライトとシェーダー両方に）
    mLightingManager->SetLightDirection(Vector3(-mSunDir.x, -mSunDir.y, -mSunDir.z), Vector3::Zero);
   
   
    float timeOfDay = fmod(mTime, 1.0f);

    // 昼と夜の割合（昼：0.0〜1.0、夜：1.0〜0.0）
    float dayStrength = SmoothStep(0.15f, 0.25f, timeOfDay) *
                       (1.0f - SmoothStep(0.75f, 0.85f, timeOfDay));
    float nightStrength = 1.0f - dayStrength;

    // 天気による減衰（晴れ：1.0 → 嵐：0.3）
    float weatherDim = 1.0f;
    switch (mWeatherType)
    {
        case WeatherType::CLEAR:  weatherDim = 1.0f; break;
        case WeatherType::CLOUDY: weatherDim = 0.7f; break;
        case WeatherType::RAIN:   weatherDim = 0.5f; break;
        case WeatherType::STORM:  weatherDim = 0.3f; break;
        case WeatherType::SNOW:   weatherDim = 0.6f; break;
    }

    // ライトの色（太陽または月）を補間し、天気で減衰
    Vector3 sunColor  = Vector3(1.0f, 0.95f, 0.8f);
    Vector3 moonColor = Vector3(0.3f, 0.4f, 0.6f);
    Vector3 finalLightColor = (sunColor * dayStrength + moonColor * nightStrength) * weatherDim;
    mLightingManager->SetLightDiffuseColor(finalLightColor);


    // Ambientカラーも補間しつつ、天気で減衰
    Vector3 dayAmbient   = Vector3(0.7f, 0.7f, 0.7f);
    Vector3 nightAmbient = Vector3(0.1f, 0.15f, 0.2f);
    Vector3 finalAmbient = (dayAmbient * dayStrength + nightAmbient * nightStrength) * weatherDim;
    mLightingManager->SetAmbientColor(finalAmbient);
}
*/


Vector3 WeatherDomeComponent::GetSkyColor(float time)
{
    Vector3 night(0.01f, 0.02f, 0.05f);
    Vector3 day  (0.7f, 0.8f, 1.0f);
    Vector3 dusk (0.9f, 0.4f, 0.2f);

    time = fmodf(time, 1.0f);
    if (time < 0.2f)
    {
        return Vector3::Lerp(night, dusk, (time - 0.0f) / (0.2f - 0.0f));
    }
    else if (time < 0.4f)
    {
        return Vector3::Lerp(dusk, day, (time - 0.2f) / (0.4f - 0.2f));
    }
    else if (time < 0.6f)
    {
        return day;
    }
    else if (time < 0.8f)
    {
        return Vector3::Lerp(day, dusk, (time - 0.6f) / (0.8f - 0.6f));
    }
    else
    {
        return Vector3::Lerp(dusk, night, (time - 0.8f) / (1.0f - 0.8f));
    }
}

Vector3 WeatherDomeComponent::GetCloudColor(float time)
{
    Vector3 dayColor   (1.0f, 1.0f, 1.0f);
    Vector3 duskColor  (0.5f, 0.2f, 0.2f);
    Vector3 nightColor (0.001f, 0.001f, 0.0015f);

    time = fmodf(time, 1.0f);
    if (time < 0.2f)
        return Vector3::Lerp(nightColor, duskColor, (time - 0.0f) / (0.2f - 0.0f));
    else if (time < 0.4f)
        return Vector3::Lerp(duskColor, dayColor, (time - 0.2f) / (0.4f - 0.2f));
    else if (time < 0.6f)
        return dayColor;
    else if (time < 0.8f)
        return Vector3::Lerp(dayColor, duskColor, (time - 0.6f) / (0.8f - 0.6f));
    else
        return Vector3::Lerp(duskColor, nightColor, (time - 0.9f) / (1.0f - 0.9f));
}


void WeatherDomeComponent::ApplyTime()
{
    float timeOfDay = fmod(mTime, 1.0f);

    // --- 太陽方向 ---
    float angle = Math::Pi * timeOfDay; // 0.0〜π

    mSunDir = Vector3(
        -cosf(angle),
        -sinf(angle),
        0.5f * sinf(angle)
    );
    mSunDir.Normalize();

    mLightingManager->SetLightDirection(
        Vector3(-mSunDir.x, -mSunDir.y, -mSunDir.z),
        Vector3::Zero
    );

    // --- 空のベース色（シェーダと共通のロジック） ---
    mRawSkyColor   = GetSkyColor(timeOfDay);
    mRawCloudColor = GetCloudColor(timeOfDay);

    // --- 昼夜の強さ ---
    float dayStrength = SmoothStep(0.15f, 0.25f, timeOfDay) *
                       (1.0f - SmoothStep(0.75f, 0.85f, timeOfDay));
    float nightStrength = 1.0f - dayStrength;

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

    // --- ライト色 ---
    Vector3 sunColor  = Vector3(1.0f, 0.95f, 0.8f);
    Vector3 moonColor = Vector3(0.3f, 0.4f, 0.6f);
    Vector3 finalLightColor =
        (sunColor * dayStrength + moonColor * nightStrength) * weatherDim;
    mLightingManager->SetLightDiffuseColor(finalLightColor);

    // --- アンビエント色 ---
    Vector3 dayAmbient   = Vector3(0.7f, 0.7f, 0.7f);
    Vector3 nightAmbient = Vector3(0.1f, 0.15f, 0.2f);
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
