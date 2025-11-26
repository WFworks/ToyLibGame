#pragma once
#include "CameraComponent.h"
#include "MathUtils.h"

class OrbitCameraComponent : public CameraComponent
{
public:
    OrbitCameraComponent(class Actor* owner);

	void Update(float deltaTime) override;

    float GetPitchSpeed() const { return mPitchSpeed; }
    float GetYawSpeed() const { return mYawSpeed; }

    void SetPitchSpeed(float speed) { mPitchSpeed = speed; }
    void SetYawSpeed(float speed) { mYawSpeed = speed; }
    
    void ChangeHeight(float val) { mChangeOffset = val;}
    
    void ProcessInput( const struct InputState& state ) override;


private:
	// オフセット
	Vector3 mOffset;
	// Up vector（Y軸）
	Vector3 mUpVector;
    
    float mChangeOffset;
	// ピッチ回転のスピード（X軸）
	float mPitchSpeed;
	// ヨー回転のスピード（U軸）
	float mYawSpeed;

};
