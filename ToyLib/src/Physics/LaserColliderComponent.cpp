#include "Physics/LaserColliderComponent.h"
#include "Engine/Core/Actor.h"

namespace toy {

LaserColliderComponent::LaserColliderComponent(Actor* a)
: ColliderComponent(a)
, mLength(100.0f) // デフォルトの射程
{
    SetFlags(C_LASER);
}

Ray LaserColliderComponent::GetRay() const
{
    Vector3 start = GetOwner()->GetPosition();
    Vector3 dir = GetOwner()->GetForward(); // 進行方向に伸びる
    return Ray(start, dir);
}

} // namespace toy
