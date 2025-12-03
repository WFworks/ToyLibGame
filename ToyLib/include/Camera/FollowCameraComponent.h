#pragma once

#include "Camera/CameraComponent.h"

namespace toy {

// SpringUtil.h とかにまとめるイメージ
struct SpringSettings
{
    float Stiffness   = 2000.0f;  // バネ定数 k
    float DampingRatio = 1.0f;    // 減衰比 ζ (1.0 で臨界減衰)
};
inline void UpdateSpring(
                         Vector3& position,
                         Vector3& velocity,
                         const Vector3& target,
                         const SpringSettings& settings,
                         float deltaTime)
{
    const float k  = settings.Stiffness;
    const float z  = settings.DampingRatio;
    const float c  = 2.0f * z * Math::Sqrt(k);  // c = 2 ζ √k（質量1前提）
    
    // x = 現在 - 目標
    Vector3 diff = position - target;
    // a = -kx - c v
    Vector3 accel = -k * diff - c * velocity;
    
    velocity += accel * deltaTime;
    position += velocity * deltaTime;
}

class FollowCameraComponent : public CameraComponent
{
public:
    FollowCameraComponent(Actor* owner);
    
    void Update(float deltaTime) override;
    void SnapToIdeal();
    
    // 各種パラメータのアクセサ（必要に応じて）
    void SetDistance(float horz, float vert)  { mHorzDist = horz; mVertDist = vert; }
    void SetTargetDistance(float dist)        { mTargetDist = dist; }
    void SetSpringSettings(const SpringSettings& s) { mSpring = s; }
    
private:
    Vector3 ComputeCameraPos() const;
    
    float mHorzDist;     // 所有アクター後方への距離
    float mVertDist;     // カメラの高さオフセット
    float mTargetDist;   // 注視点を所有アクターから前方にどれだけ離すか
    
    SpringSettings mSpring;
    Vector3 mActualPos;
    Vector3 mVelocity;
    
    bool mFirstUpdate = true;
};

} // namespace toy
