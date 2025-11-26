#include "InertiaMoveComponent.h"
#include "Actor.h"
#include "MathUtils.h"

InertiaMoveComponent::InertiaMoveComponent(class Actor* owner, int updateOrder)
    : MoveComponent(owner, updateOrder)
    , mTargetForwardSpeed(0.0f)
    , mTargetRightSpeed(0.0f)
    , mTargetVerticalSpeed(0.0f)
    , mTargetAngularSpeed(0.0f)
    , mAcceleration(5.0f)           // 好みに応じて調整
    , mAngularAcceleration(90.0f)   // 好みに応じて調整
{
}

void InertiaMoveComponent::Update(float deltaTime)
{
    // 徐々に現在速度を目標速度に近づける
    float newForward = Math::Lerp(GetForwardSpeed(), mTargetForwardSpeed, mAcceleration * deltaTime);
    float newRight   = Math::Lerp(GetRightSpeed(), mTargetRightSpeed, mAcceleration * deltaTime);
    float newVert    = Math::Lerp(GetVerticalSpeed(), mTargetVerticalSpeed, mAcceleration * deltaTime);
    float newAngular = Math::Lerp(GetAngularSpeed(), mTargetAngularSpeed, mAngularAcceleration * deltaTime);

    SetForwardSpeed(newForward);
    SetRightSpeed(newRight);
    SetVerticalSpeed(newVert);
    SetAngularSpeed(newAngular);

    // 通常の移動処理は親クラスに任せる
    MoveComponent::Update(deltaTime);
}
