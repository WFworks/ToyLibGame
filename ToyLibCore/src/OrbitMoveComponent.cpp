#include "OrbitMoveComponent.h"
#include "Actor.h"
#include <cmath>

OrbitMoveComponent::OrbitMoveComponent(class Actor* owner, int updateOrder)
: MoveComponent(owner, updateOrder)
, mCenterActor(nullptr)
, mOrbitRadius(300.0f)
, mOrbitSpeed(45.0f) // degrees per second
, mCurrentAngle(0.0f)
{
}

void OrbitMoveComponent::Update(float deltaTime)
{
    if (mCenterActor)
    {
        mCurrentAngle += mOrbitSpeed * deltaTime;
        if (mCurrentAngle > 360.0f)
            mCurrentAngle -= 360.0f;

        float radians = Math::ToRadians(mCurrentAngle);
        Vector3 centerPos = mCenterActor->GetPosition();
        float newX = centerPos.x + mOrbitRadius * cosf(radians);
        float newZ = centerPos.z + mOrbitRadius * sinf(radians);

        mOwnerActor->SetPosition(Vector3(newX, mOwnerActor->GetPosition().y, newZ));

        Vector3 toCenter = centerPos - mOwnerActor->GetPosition();
        toCenter.Normalize();
        float angle = atan2f(toCenter.x, toCenter.z);
        mOwnerActor->SetRotation(Quaternion(Vector3::UnitY, angle));
    }
}
