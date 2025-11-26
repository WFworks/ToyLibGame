#include "FPSMoveComponent.h"
#include "InputSystem.h"
#include "Actor.h"


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
        
    if (state.Keyboard.GetKeyState(SDL_SCANCODE_LEFT) == EHeld)
    {
        mAngularSpeed = -mTurnSpeed;
    }
    if (state.Keyboard.GetKeyState(SDL_SCANCODE_RIGHT) == EHeld)
    {
        mAngularSpeed = mTurnSpeed;
    }
    if (state.Keyboard.GetKeyState(SDL_SCANCODE_UP) == EHeld)
    {
        mForwardSpeed = mSpeed;
    }
    if (state.Keyboard.GetKeyState(SDL_SCANCODE_DOWN) == EHeld)
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
        Quaternion rot = Quaternion::Concatenate(mOwnerActor->GetRotation(), inc);
        mOwnerActor->SetRotation(rot);
    }

    // 前進・後退（RayCCD付き）
    if (!Math::NearZero(mForwardSpeed))
    {
        Vector3 moveDir = mOwnerActor->GetForward();
        moveDir.y = 0.0f;
        if (moveDir.LengthSq() > Math::NearZeroEpsilon)
        {
            moveDir.Normalize();
            Vector3 displacement = moveDir * mForwardSpeed;
            TryMoveWithRayCheck(displacement, deltaTime);
        }
    }
}
