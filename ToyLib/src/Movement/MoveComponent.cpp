#include "Movement/MoveComponent.h"
#include "Engine/Core/Actor.h"
#include "Engine/Core/Application.h"
#include "Physics/PhysWorld.h"
#include "Physics/ColliderComponent.h"

namespace toy {

// コンストラクタ
MoveComponent::MoveComponent(class Actor* a, int updateOrder)
: Component(a, updateOrder)
, mAngularSpeed(0.0f)
, mForwardSpeed(0.0f)
, mRightSpeed(0.0f)
, mVerticalSpeed(0.0f)
, mIsMovable(true)
{
    
}

void MoveComponent::Update(float deltaTime)
{
    Quaternion rot = GetOwner()->GetRotation();
    if (!Math::NearZero(mAngularSpeed))
    {
        float angle = Math::ToRadians(mAngularSpeed * deltaTime);
        Quaternion inc(Vector3::UnitY, angle);
        rot = Quaternion::Concatenate(rot, inc);
        GetOwner()->SetRotation(rot);
    }
    
    Vector3 pos = GetOwner()->GetPosition();
    if (!Math::NearZero(mForwardSpeed))
    {
        pos += GetOwner()->GetForward() * mForwardSpeed * deltaTime;
    }
    if (!Math::NearZero(mRightSpeed))
    {
        pos += GetOwner()->GetRight() * mRightSpeed * deltaTime;
    }
    if (!Math::NearZero(mVerticalSpeed))
    {
        pos += GetOwner()->GetUpward() * mVerticalSpeed * deltaTime;
    }
    
    GetOwner()->SetPosition(pos);
}

void MoveComponent::Reset()
{
    mAngularSpeed = 0.0f;
    mForwardSpeed = 0.0f;
    mRightSpeed = 0.0f;
    mVerticalSpeed = 0.0f;
}


bool MoveComponent::TryMoveWithRayCheck(const Vector3& moveVec, float deltaTime)
{
    if (!GetOwner() || !mIsMovable) return false;
    
    Vector3 start = GetOwner()->GetPosition();
    Vector3 goal = start + moveVec * deltaTime;
    
    Vector3 stopPos;
    if (GetOwner()->GetApp()->GetPhysWorld()->RayHitWall(start, goal, stopPos))
    {
        GetOwner()->SetPosition(stopPos);
    }
    else
    {
        GetOwner()->SetPosition(goal);
    }
    
    // 念のための押し戻し（MTV）
    GetOwner()->GetApp()->GetPhysWorld()->CollideAndCallback(C_PLAYER, C_WALL, true, false);
    
    return true;
}

} // namespace toy
