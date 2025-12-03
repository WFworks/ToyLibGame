#include "Camera/OrbitCameraComponent.h"
#include "Engine/Core/Actor.h"
#include "Engine/Runtime/InputSystem.h"
#include "Engine/Core/Application.h"
#include "Physics/PhysWorld.h"
#include <cmath>
#include <cfloat>

namespace toy {

OrbitCameraComponent::OrbitCameraComponent(Actor* owner)
: CameraComponent(owner)
, mOffset(0.0f, 4.0f, -5.0f)   // ちょっと上＋後ろ
, mUpVector(Vector3::UnitY)
, mYawSpeed(0.0f)
, mDistance(0.0f)
, mTargetDistance(0.0f)
, mMinDistance(5.0f)
, mMaxDistance(20.0f)
, mMinOffsetY(-2.0f)
, mMaxOffsetY(8.0f)
, mHeightInput(0.0f)
{
    mDistance       = mOffset.Length();
    if (mDistance < mMinDistance) mDistance = mMinDistance;
    if (mDistance > mMaxDistance) mDistance = mMaxDistance;
    mTargetDistance = mDistance;
    
    if (mOffset.y < mMinOffsetY) mOffset.y = mMinOffsetY;
    if (mOffset.y > mMaxOffsetY) mOffset.y = mMaxOffsetY;
}

void OrbitCameraComponent::ProcessInput(const InputState& state)
{
    // 時間はここでは使わない（Updateで掛ける）
    
    const float yawSpeedBase = Math::ToRadians(120.0f);
    
    float yawInput    = 0.0f;
    float heightInput = 0.0f;   // 上を +1 とする
    
    // 右スティック
    //const Vector2 rs = state.Controller.GetRightStick();
    //yawInput   += rs.x;
    //heightInput += -rs.y;   // 上を + にしたいので反転
    
    // キーボード
    if (state.IsButtonDown(GameButton::KeyD))
    {
        yawInput += 1.0f;
    }
    if (state.IsButtonDown(GameButton::KeyA))
    {
        yawInput -= 1.0f;
    }
    if (state.IsButtonDown(GameButton::KeyS))
    {
        heightInput += 1.0f;   // 上
    }
    if (state.IsButtonDown(GameButton::KeyW))
    {
        heightInput -= 1.0f;   // 下
    }
    
    // 実際の角速度に変換
    mYawSpeed    = yawInput * yawSpeedBase;
    mHeightInput = heightInput;
}

void OrbitCameraComponent::Update(float deltaTime)
{
    CameraComponent::Update(deltaTime);
    
    //----------------------
    // 1. ヨー回転（水平公転）
    //----------------------
    Quaternion yawRot(Vector3::UnitY, mYawSpeed * deltaTime);
    mOffset   = Vector3::Transform(mOffset, yawRot);
    mUpVector = Vector3::Transform(mUpVector, yawRot);
    
    //----------------------
    // 2. 高さの更新（距離は高さから決める）
    //----------------------
    const float heightSpeed = 7.0f;  // 高さの速さ（調整ポイント）
    
    if (std::fabs(mHeightInput) > 1e-4f)
    {
        // 高さを直接オフセットYに反映
        mOffset.y += mHeightInput * heightSpeed * deltaTime;
        
        // 高さクランプ
        if (mOffset.y < mMinOffsetY) mOffset.y = mMinOffsetY;
        if (mOffset.y > mMaxOffsetY) mOffset.y = mMaxOffsetY;
    }
    
    // 次フレームのために入力をリセット
    mHeightInput = 0.0f;
    
    // ここで「高さから距離を決定」する
    //   低い（mMinOffsetY） → 近い（mMinDistance）
    //   高い（mMaxOffsetY） → 遠い（mMaxDistance）
    float t = (mOffset.y - mMinOffsetY) / (mMaxOffsetY - mMinOffsetY);
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    
    float nearDist = mMinDistance;
    float farDist  = mMaxDistance;
    mTargetDistance = nearDist + (farDist - nearDist) * t;
    
    //----------------------
    // 3. 距離をターゲットに寄せる
    //----------------------
    const float zoomLerpSpeed = 10.0f; // 追従の速さ（大きいほどキビキビ）
    mDistance += (mTargetDistance - mDistance) * zoomLerpSpeed * deltaTime;
    
    if (mDistance < mMinDistance) mDistance = mMinDistance;
    if (mDistance > mMaxDistance) mDistance = mMaxDistance;
    
    // オフセット方向を正規化し、距離を反映
    Vector3 dir = mOffset;
    dir.Normalize();
    mOffset = dir * mDistance;
    
    //----------------------
    // 4. カメラ位置決定
    //----------------------
    Vector3 target = GetOwner()->GetPosition() + Vector3(0.0f, 2.5f, 0.0f);
    Vector3 cameraPos = target + mOffset;
    
    //----------------------
    // 5. 地面との当たり補正
    //----------------------
    if (Application* app = GetOwner()->GetApp())
    {
        if (PhysWorld* phys = app->GetPhysWorld())
        {
            mCameraActor->SetPosition(cameraPos);
            
            float groundY = phys->GetGroundHeightAt(mCameraActor->GetPosition());
            if (groundY != -FLT_MAX)
            {
                const float margin = 0.1f;
                float minY = groundY + margin;
                
                if (cameraPos.y < minY)
                {
                    cameraPos.y = minY;
                }
            }
        }
    }
    
    
    //----------------------
    // 6. 補正された位置から内部状態を同期
    //----------------------
    Vector3 toCam = cameraPos - target;
    mOffset = toCam;
    
    // 距離再計算
    mDistance = mOffset.Length();
    if (mDistance < mMinDistance) mDistance = mMinDistance;
    if (mDistance > mMaxDistance) mDistance = mMaxDistance;
    
    // 高さも再クランプ
    if (mOffset.y < mMinOffsetY) mOffset.y = mMinOffsetY;
    if (mOffset.y > mMaxOffsetY) mOffset.y = mMaxOffsetY;
    
    // オフセットを距離付きで再構築
    Vector3 n = mOffset;
    n.Normalize();
    mOffset = n * mDistance;
    cameraPos = target + mOffset;
    
    //----------------------
    // 7. ビュー行列セット
    //----------------------
    mCameraPosition = cameraPos;
    
    Matrix4 view = Matrix4::CreateLookAt(
                                         cameraPos,
                                         target,
                                         mUpVector
                                         );
    SetViewMatrix(view);
    
    mCameraActor->SetPosition(cameraPos);
}

} // namespace toy
