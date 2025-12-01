#include "Camera/OrbitCameraComponent.h"
#include "Engine/Core/Actor.h"
#include "Physics/ColliderComponent.h"
#include "Engine/Runtime/InputSystem.h"
#include "Physics/PhysWorld.h"
#include "Engine/Core/Application.h"

OrbitCameraComponent::OrbitCameraComponent(Actor* actor)
: CameraComponent(actor)
, mOffset(-0.0f, 4.0f, -5.0f)
, mUpVector(Vector3::UnitY)
, mPitchSpeed(0.0f)
, mYawSpeed(0.0f)
{
    
}

void OrbitCameraComponent::ProcessInput( const struct InputState& state )
{
    float angularSpeed = 2.0f;
    
    SetYawSpeed( angularSpeed * state.Controller.GetRightStick().x );
    ChangeHeight( angularSpeed * state.Controller.GetRightStick().y );


    // キーボード（カメラ）
    if (state.Keyboard.GetKeyState(SDL_SCANCODE_D) == EHeld)
    {
        SetYawSpeed( -angularSpeed );
    }
    if (state.Keyboard.GetKeyState(SDL_SCANCODE_A) == EHeld)
    {
        SetYawSpeed( angularSpeed );
    }
    if (state.Keyboard.GetKeyState(SDL_SCANCODE_W) == EHeld)
    {
        ChangeHeight(-0.2);
    }
    if (state.Keyboard.GetKeyState(SDL_SCANCODE_S) == EHeld)
    {
        ChangeHeight(0.2);
    }
    

}

void OrbitCameraComponent::Update(float deltaTime)
{
    CameraComponent::Update(deltaTime);

    // --- いつもの回転処理 ---
    Quaternion yaw(Vector3::UnitY, mYawSpeed * deltaTime);
    mOffset   = Vector3::Transform(mOffset, yaw);
    mUpVector = Vector3::Transform(mUpVector, yaw);

    Vector3 forward = -1.0f * mOffset;
    forward.Normalize();
    Vector3 right = Vector3::Cross(mUpVector, forward);
    right.Normalize();

    Quaternion pitch(right, mPitchSpeed * deltaTime);
    mOffset   = Vector3::Transform(mOffset, pitch);
    mUpVector = Vector3::Transform(mUpVector, pitch);

    // 高さの手動調整
    mOffset.y += mChangeOffset;

    Vector3 target    = GetOwner()->GetPosition() + Vector3(0.0f, 2.5f, 0.0f);
    Vector3 cameraPos = target + mOffset;

    //========================
    // 地面との当たり補正
    //========================
    Application* app = GetOwner()->GetApp();
    if (app)
    {
        PhysWorld* phys = app->GetPhysWorld();
        
        // まずカメラActorの仮の位置を更新しておく
        mCameraActor->SetPosition(cameraPos);
        
        float groundY = phys->GetGroundHeightAt(mCameraActor->GetPosition());
        if (groundY != -FLT_MAX)
        {
            const float margin = 0.5f; // 地面から少しだけ浮かせる
            float minY = groundY + margin;
            
            if (cameraPos.y < minY)
            {
                cameraPos.y = minY;
                
                // ユーザー入力による縦移動をちょっと打ち消しておくならここ
                if (mChangeOffset < 0.0f)
                {
                    mOffset.y -= mChangeOffset;
                }
            }
        }
    }

    //========================

    mCameraPosition = cameraPos;

    Matrix4 view = Matrix4::CreateLookAt(cameraPos, target, mUpVector);
    SetViewMatrix(view);

    // 最終的な位置をActorにも反映
    mCameraActor->SetPosition(cameraPos);
   
}
