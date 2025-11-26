#pragma once

#include "MoveComponent.h"
#include "MathUtils.h"

class InertiaMoveComponent : public MoveComponent
{
public:
    InertiaMoveComponent(class Actor* owner, int updateOrder = 10);

    void Update(float deltaTime) override;

    // 目標速度をセット
    void SetTargetForwardSpeed(float speed) { mTargetForwardSpeed = speed; }
    void SetTargetRightSpeed(float speed) { mTargetRightSpeed = speed; }
    void SetTargetVerticalSpeed(float speed) { mTargetVerticalSpeed = speed; }
    void SetTargetAngularSpeed(float speed) { mTargetAngularSpeed = speed; }

    // 加速度を設定
    void SetAcceleration(float acc) { mAcceleration = acc; }
    void SetAngularAcceleration(float acc) { mAngularAcceleration = acc; }

private:
    float mTargetForwardSpeed;
    float mTargetRightSpeed;
    float mTargetVerticalSpeed;
    float mTargetAngularSpeed;

    float mAcceleration;         // 線形速度加速度
    float mAngularAcceleration;  // 角速度加速度
};
