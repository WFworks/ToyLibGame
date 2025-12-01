#include "Movement/FPSMoveComponent.h"
#include "Engine/Runtime/InputSystem.h"
#include "Engine/Core/Actor.h"


FPSMoveComponent::FPSMoveComponent(class Actor* a, int order)
: MoveComponent(a, order)
, mTurnSpeed(180.f)
, mSpeed(7.f)
{
    
}

FPSMoveComponent::~FPSMoveComponent()
{
    
}


void FPSMoveComponent::ProcessInput(const struct InputState& state)
{

    if(!mIsMovable) return;

    mForwardSpeed = mSpeed * state.Controller.GetLeftStick().y;
    mAngularSpeed = mTurnSpeed * state.Controller.GetLeftStick().x;
        
    if (state.IsButtonDown(GameButton::DPadLeft))
    {
        mAngularSpeed = -mTurnSpeed;
    }
    if (state.IsButtonDown(GameButton::DPadRight))
    {
        mAngularSpeed = mTurnSpeed;
    }
    if (state.IsButtonDown(GameButton::DPadUp))
    {
        mForwardSpeed = mSpeed;
    }
    if (state.IsButtonDown(GameButton::DPadDown))
    {
        mForwardSpeed = -mSpeed;
    }
}

void FPSMoveComponent::Update(float deltaTime)
{
    // 回転（Y軸）
    if (!Math::NearZero(mAngularSpeed))
    {
        float angle = Math::ToRadians(mAngularSpeed * deltaTime);
        Quaternion inc(Vector3::UnitY, angle);
        Quaternion rot = Quaternion::Concatenate(GetOwner()->GetRotation(), inc);
        GetOwner()->SetRotation(rot);
    }

    // 前進・後退（RayCCD付き）
    if (!Math::NearZero(mForwardSpeed))
    {
        Vector3 moveDir = GetOwner()->GetForward();
        moveDir.y = 0.0f;
        if (moveDir.LengthSq() > Math::NearZeroEpsilon)
        {
            moveDir.Normalize();
            Vector3 displacement = moveDir * mForwardSpeed;
            TryMoveWithRayCheck(displacement, deltaTime);
        }
    }
}
