#include "GravityComponent.h"
#include "Actor.h"
#include "ColliderComponent.h"
#include "BoundingVolumeComponent.h"
#include "PhysWorld.h"
#include "Application.h"

GravityComponent::GravityComponent(Actor* a)
: Component(a)
, mVelocityY(0.0f)
, mGravityAccel(-2.8f)
, mJumpSpeed(50.0f)
, mIsGrounded(false)
{
}

void GravityComponent::Update(float deltaTime)
{
    //if (mIsGrounded) return;
    // 重力加速度を加算
    mVelocityY += mGravityAccel;
    // 現在座標を取得
    Vector3 pos = mOwnerActor->GetPosition();
    // 設置判定に使うコライダーを取得（C_FOOT）
    ColliderComponent* collider = FindFootCollider();
    if (!collider) return;


    // 地面の高さを取得
    float groundY = -FLT_MAX;
    if (mOwnerActor->GetApp()->GetPhysWorld()
        ->GetNearestGroundY(mOwnerActor, groundY))
    {
        // 自分より下に地面がある
        // AABBから最低座標を取得
        float footY = collider->GetBoundingVolume()->GetWorldAABB().min.y;
        if (footY+mVelocityY*deltaTime < groundY)
        {
            float offset = pos.y - footY;
            pos.y = groundY + offset+0.01;
            mVelocityY = 0.0f;
            mIsGrounded = true;
            mOwnerActor->SetPosition(pos);
            return;
        }
    }
    else
    {
        // 自分より下に地面がない
        mIsGrounded = false;
    }
    pos.y += mVelocityY * deltaTime;
    mOwnerActor->SetPosition(pos);
}


void GravityComponent::Jump()
{
    if (mIsGrounded)
    {
        mVelocityY = mJumpSpeed;
        mIsGrounded = false;
    }
}
ColliderComponent* GravityComponent::FindFootCollider()
{
    for (auto* comp : mOwnerActor->GetAllComponents<ColliderComponent>())
    {
        if (comp->HasFlag(C_FOOT))
        {
            return comp;
        }
    }
    return nullptr;
}


