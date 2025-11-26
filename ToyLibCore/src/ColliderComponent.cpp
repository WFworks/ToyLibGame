#include "ColliderComponent.h"
#include "Actor.h"
#include "BoundingVolumeComponent.h"
#include "Application.h"
#include "PhysWorld.h"

#include <algorithm>

// コンストラクタ
ColliderComponent::ColliderComponent(Actor* a)
: Component(a)
, mFlags(C_NONE)
, mIsCollided(false)
, mIsDisp(true)
//, targetType(C_NONE)
{
    mBoundingVolume = mOwnerActor->CreateComponent<BoundingVolumeComponent>();
    mOwnerActor->GetApp()->GetPhysWorld()->AddCollider(this);
}

ColliderComponent::~ColliderComponent()
{
    mOwnerActor->GetApp()->GetPhysWorld()->RemoveCollider(this);
}

void ColliderComponent::Update(float deltaTime)
{
    //mTargetColliders.clear();
}


// 衝突した
void ColliderComponent::Collided(ColliderComponent* c)
{
    if (std::find(mTargetColliders.begin(), mTargetColliders.end(), c) == mTargetColliders.end())
    {
        mTargetColliders.emplace_back(c);
        mIsCollided = true;
    }
}
