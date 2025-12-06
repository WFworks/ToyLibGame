#include "Physics/GravityComponent.h"
#include "Engine/Core/Actor.h"
#include "Physics/ColliderComponent.h"
#include "Physics/BoundingVolumeComponent.h"
#include "Physics/PhysWorld.h"
#include "Engine/Core/Application.h"
#include <limits>

namespace toy {

//------------------------------------------------------------------------------
// コンストラクタ
//------------------------------------------------------------------------------
// ・初期Y速度は 0。
// ・重力加速度はデフォルトで -2.8f（ゲーム全体で調整する前提）。
// ・ジャンプ初速は 50.0f。
//------------------------------------------------------------------------------
GravityComponent::GravityComponent(Actor* a)
: Component(a)
, mVelocityY(0.0f)
, mGravityAccel(-2.8f)
, mJumpSpeed(50.0f)
, mIsGrounded(false)
{
}

//------------------------------------------------------------------------------
// Update
//------------------------------------------------------------------------------
// ・mVelocityY に重力加速度を加算して位置更新。
// ・PhysWorld::GetNearestGroundY() を使って足元の最も近い地面Yを取得。
// ・足コライダー（C_FOOT）AABB の min.y と groundY を比較して、
//   次フレームの足元が groundY を下回る場合は接地とみなし、
//   Y位置を補正して mVelocityY を 0 / mIsGrounded = true にする。
//------------------------------------------------------------------------------
void GravityComponent::Update(float deltaTime)
{
    // 重力加速度を加算（毎フレーム下向きに加速）
    mVelocityY += mGravityAccel;
    
    // 現在の Actor 座標
    Vector3 pos = GetOwner()->GetPosition();
    
    // 設置判定に使う足コライダーを取得（C_FOOT を持つ Collider）
    ColliderComponent* collider = FindFootCollider();
    if (!collider) return;
    
    // 足元より下方向の中で「最も近い地面の高さ」を取得
    float groundY = -std::numeric_limits<float>::max();
    if (GetOwner()->GetApp()->GetPhysWorld()->GetNearestGroundY(GetOwner(), groundY))
    {
        // 自分の真下に何らかの地面候補が存在する
        
        // 足コライダーのワールドAABBから「足元のY」を取得
        float footY = collider->GetBoundingVolume()->GetWorldAABB().min.y;
        
        // このフレームの更新後に足元が地面を突き抜けるかをチェック
        if (footY + mVelocityY * deltaTime < groundY)
        {
            // 足元が groundY を下回ってしまうので、ぴったり乗るように補正
            float offset = pos.y - footY;             // Actorの原点から足元までのオフセット
            pos.y = groundY + offset + 0.01f;         // ほんの少し浮かせてめり込み防止
            mVelocityY = 0.0f;                        // 落下速度リセット
            mIsGrounded = true;                       // 接地状態
            GetOwner()->SetPosition(pos);
            return;
        }
    }
    else
    {
        // 自分より下に地面が見つからない → 空中扱い
        mIsGrounded = false;
    }
    
    // 通常の落下処理（地面に当たらなかった場合）
    pos.y += mVelocityY * deltaTime;
    GetOwner()->SetPosition(pos);
}

//------------------------------------------------------------------------------
// Jump
//------------------------------------------------------------------------------
// ・接地中（mIsGrounded == true）のときだけジャンプ初速を与える。
// ・ジャンプ後は mIsGrounded を false にする。
//------------------------------------------------------------------------------
void GravityComponent::Jump()
{
    if (mIsGrounded)
    {
        mVelocityY = mJumpSpeed;
        mIsGrounded = false;
    }
}

//------------------------------------------------------------------------------
// FindFootCollider
//------------------------------------------------------------------------------
// ・Actor に紐づく ColliderComponent 群から、C_FOOT フラグを持つものを探す。
// ・足用 Collider（カプセルや小さめAABB）を用意しておき、
//   それを地面判定専用に使う前提のヘルパー。
//------------------------------------------------------------------------------
ColliderComponent* GravityComponent::FindFootCollider()
{
    for (auto* comp : GetOwner()->GetAllComponents<ColliderComponent>())
    {
        if (comp->HasFlag(C_FOOT))
        {
            return comp;
        }
    }
    return nullptr;
}

} // namespace toy
