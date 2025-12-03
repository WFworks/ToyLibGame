
#include "Camera/FollowCameraComponent.h"
#include "Engine/Core/Actor.h"

namespace toy {

FollowCameraComponent::FollowCameraComponent(Actor* owner)
: CameraComponent(owner)
, mHorzDist(10.0f)
, mVertDist(4.0f)
, mTargetDist(10.0f)
, mSpring{ 2000.0f, 1.0f } // k=2000, ζ=1.0（臨界減衰）
, mActualPos(Vector3::Zero)
, mVelocity(Vector3::Zero)
, mFirstUpdate(true)
{
}

void FollowCameraComponent::Update(float deltaTime)
{
    CameraComponent::Update(deltaTime);
    
    // 最初の1回はスナップしておくと安全
    if (mFirstUpdate)
    {
        SnapToIdeal();
        mFirstUpdate = false;
    }
    
    // 理想位置を計算
    Vector3 idealPos = ComputeCameraPos();
    
    // スプリングで mActualPos を idealPos に近づける
    UpdateSpring(mActualPos, mVelocity, idealPos, mSpring, deltaTime);
    
    // 注視点（所有アクターの前方）
    Vector3 target = GetOwner()->GetPosition()
    + GetOwner()->GetForward() * mTargetDist;
    
    // ビュー行列
    Matrix4 view = Matrix4::CreateLookAt(mActualPos, target, Vector3::UnitY);
    SetViewMatrix(view);
    SetCameraPosition(mActualPos);
}

void FollowCameraComponent::SnapToIdeal()
{
    mActualPos = ComputeCameraPos();
    mVelocity  = Vector3::Zero;
    
    Vector3 target = GetOwner()->GetPosition()
    + GetOwner()->GetForward() * mTargetDist;
    
    Matrix4 view = Matrix4::CreateLookAt(mActualPos, target, Vector3::UnitY);
    SetViewMatrix(view);
    SetCameraPosition(mActualPos);
}

Vector3 FollowCameraComponent::ComputeCameraPos() const
{
    Vector3 cameraPos = GetOwner()->GetPosition();
    cameraPos -= GetOwner()->GetForward() * mHorzDist;
    cameraPos += Vector3::UnitY * mVertDist;
    return cameraPos;
}

} // namespace toy
