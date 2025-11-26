#include "LaserColliderComponent.h"
#include "Actor.h"

LaserColliderComponent::LaserColliderComponent(Actor* a)
: ColliderComponent(a)
, mLength(100.0f) // デフォルトの射程
{
    SetFlags(C_LASER);
}

Ray LaserColliderComponent::GetRay() const
{
    Vector3 start = mOwnerActor->GetPosition();
    Vector3 dir = mOwnerActor->GetForward(); // 進行方向に伸びる
    return Ray(start, dir);
}
