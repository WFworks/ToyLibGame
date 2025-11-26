#pragma once
#include "MoveComponent.h"

class DirMoveComponent : public MoveComponent
{
public:
    DirMoveComponent(class Actor* owner, int updateOrder = 10);
    virtual ~DirMoveComponent();
    void Update(float deltTime) override;
    void ProcessInput( const struct InputState& state ) override;
private:
    float mSpeed;
    void AdjustDir();
    Vector3 mPrevPosition;
};
