#pragma once

#include "CameraComponent.h"


// Actorについていくカメラ
class FollowCameraComponent : public CameraComponent
{
public:
    // コンストラクタ
    FollowCameraComponent(class Actor* owner);

    // Override
	void Update(float deltaTime) override;
    // 理想の位置に向けて調整
	void SnapToIdeal();

    // カメラ位置の設定
    void SetHorzDist(float dist) { mHorzDist = dist; }
    void SetVertDist(float dist) { mVertDist = dist; }
    void SetTargetDist(float dist) { mTargetDist = dist; }
    void SetSpringConstant(float spring) { mSpringConstant = spring; }
    
private:
    // カメラの位置を計算
	Vector3 ComputeCameraPos() const;
    // 現在の位置
	Vector3 mActualPos;
    // 速度
	Vector3 mVelocity;
    
    // カメラ位置
    // 後ろ
	float mHorzDist;
    // 高さ
	float mVertDist;
    // 視点とActorの距離
	float mTargetDist;
    
    // バネ定数
	float mSpringConstant;
};

