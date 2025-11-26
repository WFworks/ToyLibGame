#pragma once
#include "MathUtils.h"
#include <memory>

// ディレクショナルライト
struct DirectionalLight
{
    Vector3 Target = Vector3::Zero;
    Vector3 Position = Vector3::UnitZ;
    Vector3 DiffuseColor = Vector3(0.5f, 0.5f, 0.5f);   // 色
    Vector3 SpecColor = Vector3(0.5f, 0.5f, 0.5f);      // 反射色
    
    Vector3 GetDirection() const { return Vector3::Normalize(Target - Position); }
    // PositionとTargetを一括設定
    void LookAt(const Vector3& position, const Vector3& target) {
        Position = position;
        Target = target;
    }
};

struct FogInfo
{
    float MaxDist = 10000.f;
    float MinDist = 0.00001f;
    Vector3 Color = Vector3(0.5f, 0.5f, 0.5f);
};

class LightingManager
{
public:
    LightingManager();
    ~LightingManager();
    
    // 構造体でのセット
    void SetDirectionalLight(const DirectionalLight& light) { mDirLight = light; }
    const DirectionalLight& GetDirectionalLight() const { return mDirLight; }
    void SetFogInfo(const FogInfo& fog) { mFog = fog; }
    const FogInfo& GetFogInfo() const { return mFog; }
    
    // 個別パラメーター設定
    // Directional Light
    void SetLightPosition(const Vector3& pos)        { mDirLight.Position = pos; }
    void SetLightTarget(const Vector3& target)       { mDirLight.Target = target; }
    void SetLightDirection(const Vector3& pos, const Vector3& target) { mDirLight.LookAt(pos, target); }
    void SetLightDiffuseColor(const Vector3& color) { mDirLight.DiffuseColor = color; }
    void SetLightSpecColor(const Vector3& color)    { mDirLight.SpecColor = color; }
    Vector3 GetLightPosition() const                { return mDirLight.Position; }
    Vector3 GetLightDirection() const               { return mDirLight.GetDirection(); }
    // Ambient Color
    void SetAmbientColor(const Vector3 color)       { mAmbientColor = color; }
    const Vector3& GetAmbientColor()                { return mAmbientColor; };
    // Fog
    void SetFogMinDist(float min)                   { mFog.MinDist = min; }
    void SetFogMaxDist(float max)                   { mFog.MaxDist = max; }
    void SetFogColor(const Vector3& color)          { mFog.Color = color; }

    float GetFogMinDist() const                     { return mFog.MinDist; }
    float GetFogMaxDist() const                     { return mFog.MaxDist; }
    const Vector3& GetFogColor() const              { return mFog.Color; }


    // シェーダーに反映
    void ApplyToShader(std::shared_ptr<class Shader> shader, const Matrix4& viewMatrix);
    
private:
    DirectionalLight mDirLight;
    FogInfo mFog;
    Vector3 mAmbientColor = Vector3(0.5f, 0.5f, 0.5f);
};
