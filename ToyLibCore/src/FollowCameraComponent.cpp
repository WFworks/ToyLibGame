
#include "FollowCameraComponent.h"
#include "Actor.h"

FollowCameraComponent::FollowCameraComponent(Actor* owner)
: CameraComponent(owner)
, mHorzDist(10.0f)        // 所有アクターとの距離
, mVertDist(4.0f)        // 所有アクターとの距離（高さ）
, mTargetDist(10.0f)      // 視点は所有アクターよりどのくらい前か
, mSpringConstant(2000.0f)  // バネ定数
, mVelocity(Vector3(100,100,100))
{
    
}

void FollowCameraComponent::Update(float deltaTime)
{
	CameraComponent::Update(deltaTime);
    mCameraPosition = mActualPos;
    
	// バネ定数から減衰を計算
    float dampening = 2.0f * Math::Sqrt(mSpringConstant);
    
	// 理想の位置
	Vector3 idealPos = ComputeCameraPos();
	// 実際の位置と理想の位置の差
    Vector3 diff = mActualPos - idealPos;
	// バネによる加速度を計算
    Vector3 acel = -mSpringConstant * diff - dampening * mVelocity;
	// 加速度を更新
    mVelocity += acel * deltaTime;
	// カメラの場所を更新
    mActualPos += mVelocity * deltaTime;
    //mActualPos = idealPos;
	// ターゲットは所有アクターの前方
    Vector3 target = mOwnerActor->GetPosition() + mOwnerActor->GetForward() * mTargetDist;
    
   

    
	// ビューマトリックスを生成
    Matrix4 view = Matrix4::CreateLookAt(mActualPos, target, Vector3::UnitY);
	SetViewMatrix(view);
    
    SetCameraPosition(mActualPos);
}

// 理想の位置に向けて調整
void FollowCameraComponent::SnapToIdeal()
{
	// 実際の位置
    mActualPos = ComputeCameraPos();
	// 速度ゼロ
    mVelocity = Vector3::Zero;
	// ターゲットとビューを計算
    Vector3 target = mOwnerActor->GetPosition() + mOwnerActor->GetForward() * mTargetDist;
	// ビューマトリックスを生成
    Matrix4 view = Matrix4::CreateLookAt(mActualPos, target, Vector3::UnitY);
	SetViewMatrix(view);
    SetCameraPosition(mActualPos);
}

Vector3 FollowCameraComponent::ComputeCameraPos() const
{
	// カメラの位置を所有アクターの上のほうにセット
	Vector3 cameraPos = mOwnerActor->GetPosition();
    {
        //cameraPos.x = cameraPos.x / 2;
        //cameraPos.y = cameraPos.y / 2;
    }
    cameraPos -= mOwnerActor->GetForward() * mHorzDist;
    cameraPos += Vector3::UnitY * mVertDist;
	return cameraPos;
}
