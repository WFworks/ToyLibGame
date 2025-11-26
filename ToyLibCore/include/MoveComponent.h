#pragma once

#include "Component.h"
#include "MathUtils.h"

class MoveComponent : public Component
{
    
public:
    MoveComponent(class Actor* owner, int updateOrder = 10);
    void Update(float deltaTime) override;
    
    // Getter/Setter
    float GetAngularSpeed() const { return mAngularSpeed; }
    float GetForwardSpeed() const { return mForwardSpeed; }
    float GetRightSpeed() const { return mRightSpeed; }
    float GetVerticalSpeed() const { return mVerticalSpeed; }

    void SetAngularSpeed(float speed) { mAngularSpeed = speed; }
    void SetForwardSpeed(float speed) { mForwardSpeed = speed; }
    void SetRightSpeed(float speed) { mRightSpeed = speed;}
    void SetVerticalSpeed(float speed) { mVerticalSpeed = speed; }
    
    void SetIsMovable(const bool b) { mIsMovable = b; if(!b)Reset(); } // 既に移動量がセットされていたらリセットする
    bool GetIsMovable() const { return mIsMovable; }
    
    void SetIsTurnable(const bool b) { mIsTurnable = b; }
    bool GetIsTurnable() const { return mIsTurnable; }
    
    void Reset();
    // 壁すり抜け防止機能付きの移動
    bool TryMoveWithRayCheck(const Vector3& moveVec, float deltaTime);

protected:
    float mAngularSpeed;    // 回転
    float mForwardSpeed;    // 前後
    float mRightSpeed;      // 左右
    float mVerticalSpeed;   // 上下 （正＝上昇、負＝落下）
    
    bool mIsMovable;        // 移動可能な状態か
    bool mIsTurnable;       // 回転可能な状態か

};
