#pragma once

#include "MoveComponent.h"

class FollowMoveComponent : public MoveComponent
{
public:
    FollowMoveComponent(class Actor* owner, int updateOrder = 10);

    void Update(float deltaTime) override;

    void SetTarget(class Actor* target) { mTarget = target; }
    void SetFollowDistance(float dist) { mFollowDistance = dist; }
    void SetFollowSpeed(float speed) { mFollowSpeed = speed; }
    void SetRotationSpeed(float speed) { mRotationSpeed = speed; }

private:
    class Actor* mTarget;
    float mFollowDistance;
    float mFollowSpeed;
    float mRotationSpeed;
};
