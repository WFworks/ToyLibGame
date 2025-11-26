#pragma once

#include "ColliderComponent.h"
#include "Polygon.h"

class LaserColliderComponent : public ColliderComponent
{
public:
    LaserColliderComponent(class Actor* a);

    // レーザーはレイで判定する
    Ray GetRay() const override;

    void SetRayLength(float len) { mLength = len; }
    float GetRayLength() const { return mLength; }

private:
    float mLength; // レーザーの長さ（描画/判定用）
};
