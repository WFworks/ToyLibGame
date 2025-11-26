#include "MoveComponent.h"
#include "Actor.h"
#include "Application.h"
#include "PhysWorld.h"
#include "ColliderComponent.h"


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
    Quaternion rot = mOwnerActor->GetRotation();
    if (!Math::NearZero(mAngularSpeed))
    {
        float angle = Math::ToRadians(mAngularSpeed * deltaTime);
        Quaternion inc(Vector3::UnitY, angle);
        rot = Quaternion::Concatenate(rot, inc);
        mOwnerActor->SetRotation(rot);
    }

    Vector3 pos = mOwnerActor->GetPosition();
    if (!Math::NearZero(mForwardSpeed))
        pos += mOwnerActor->GetForward() * mForwardSpeed * deltaTime;

    if (!Math::NearZero(mRightSpeed))
        pos += mOwnerActor->GetRight() * mRightSpeed * deltaTime;

    if (!Math::NearZero(mVerticalSpeed))
        pos += mOwnerActor->GetUpward() * mVerticalSpeed * deltaTime;

    mOwnerActor->SetPosition(pos);
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
    if (!mOwnerActor || !mIsMovable) return false;

    Vector3 start = mOwnerActor->GetPosition();
    Vector3 goal = start + moveVec * deltaTime;

    Vector3 stopPos;
    if (mOwnerActor->GetApp()->GetPhysWorld()->RayHitWall(start, goal, stopPos))
    {
        mOwnerActor->SetPosition(stopPos);
    }
    else
    {
        mOwnerActor->SetPosition(goal);
    }

    // 念のための押し戻し（MTV）
    mOwnerActor->GetApp()->GetPhysWorld()->CollideAndCallback(C_PLAYER, C_WALL, true, false);

    return true;
}
