#include "WeatherDomeComponent.h"
#include "SkyDomeMeshGenerator.h"
#include "VertexArray.h"
#include "Renderer.h"
#include "LightingManager.h"
#include "Actor.h"
#include "Application.h"
#include "Renderer.h"
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
