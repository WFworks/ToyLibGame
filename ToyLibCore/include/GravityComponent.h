#pragma once
#include "Component.h"

class GravityComponent : public Component {
public:
    GravityComponent(class Actor* a);

    void Update(float deltaTime) override;
    void Jump();

    void SetGravityAccel(float g) { mGravityAccel = g; }
    void SetJumpSpeed(float s) { mJumpSpeed = s; }
    float GetJumpSpeed() const { return mJumpSpeed; }
    bool IsGrounded() const { return mIsGrounded; }
    
    float GetVelocityY() const { return mVelocityY; }

private:
    float mVelocityY;
    float mGravityAccel;
    float mJumpSpeed;
    bool mIsGrounded;
    
    
    class ColliderComponent* FindFootCollider();
};
