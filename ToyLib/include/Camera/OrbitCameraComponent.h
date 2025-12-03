#pragma once

#include "Camera/CameraComponent.h"
#include "Utils/MathUtil.h"

// ターゲットActorの周りを公転するカメラ
class OrbitCameraComponent : public CameraComponent
{
public:
    OrbitCameraComponent(class Actor* owner);

    void ProcessInput(const struct InputState& state) override;
    void Update(float deltaTime) override;

    float GetYawSpeed() const { return mYawSpeed; }
    void  SetYawSpeed(float speed) { mYawSpeed = speed; }

private:
    //======================
    // 基本状態
    //======================
    // ターゲットからのオフセット
    Vector3 mOffset;
    // Upベクトル（Y軸固定）
    Vector3 mUpVector;

    //======================
    // 水平方向（公転）
    //======================
    float mYawSpeed;        // 現在のヨー角速度（ラジアン/秒）

    //======================
    // 距離（ズーム）
    //======================
    float mDistance;        // 現在のカメラ距離
    float mTargetDistance;  // 目標のカメラ距離（スムーズに寄せる用）
    float mMinDistance;     // 近づける下限
    float mMaxDistance;     // 離せる上限

    //======================
    // 高さオフセット（Y）
    //======================
    float mMinOffsetY;      // 最低高さ（オフセットY）
    float mMaxOffsetY;      // 最高高さ（オフセットY）

    //======================
    // 入力状態（1フレーム分）
    //======================
    float mHeightInput;     // 上下入力（-1.0 ～ 1.0） 上を正とする
};
