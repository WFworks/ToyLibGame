#pragma once

#include "MoveComponent.h"
#include "MathUtils.h"

class OrbitMoveComponent : public MoveComponent
{
public:
    OrbitMoveComponent(class Actor* owner, int updateOrder = 10);

    void Update(float deltaTime) override;

    void SetCenterActor(class Actor* center) { mCenterActor = center; }
    void SetOrbitRadius(float radius) { mOrbitRadius = radius; }
    void SetOrbitSpeed(float speed) { mOrbitSpeed = speed; }

private:
    class Actor* mCenterActor;
    float mOrbitRadius;
    float mOrbitSpeed;
    float mCurrentAngle;
};
