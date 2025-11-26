#pragma once
#include "MoveComponent.h"

class FPSMoveComponent : public MoveComponent
{
public:
    FPSMoveComponent(class Actor* owner, int updateOrder = 10);
    virtual ~FPSMoveComponent();
    void ProcessInput( const struct InputState& state ) override;
    void Update(float deltaTime) override;
private:
    float mTurnSpeed;
    float mSpeed;
};
