#include "FollowMoveComponent.h"
#include "Actor.h"
#include "MathUtils.h"

FollowMoveComponent::FollowMoveComponent(Actor* owner, int updateOrder)
    : MoveComponent(owner, updateOrder)
    , mTarget(nullptr)
    , mFollowDistance(3.0f)
    , mFollowSpeed(200.0f)
    , mRotationSpeed(90.0f)
{
}

void FollowMoveComponent::Update(float deltaTime)
{
    MoveComponent::Update(deltaTime);

    if (mTarget)
    {
        Vector3 toTarget = mTarget->GetPosition() - mOwnerActor->GetPosition();
        float dist = toTarget.Length();

        if (dist > Math::NearZeroEpsilon)
        {
            toTarget.Normalize();

            // 向きを合わせる
            Vector3 forward = mOwnerActor->GetForward();
            float dot = Vector3::Dot(forward, toTarget);
            dot = Math::Clamp(dot, -1.0f, 1.0f);
            float angle = Math::Acos(dot);

            // 1度未満なら無視
            if (angle > Math::ToRadians(1.0f))
            {
                float maxRot = mRotationSpeed * deltaTime;
                angle = Math::Min(angle, maxRot);

                // Y軸投影で回転方向を決定
                Vector3 f2D(forward.x, 0.0f, forward.z);
                Vector3 t2D(toTarget.x, 0.0f, toTarget.z);
                f2D.Normalize();
                t2D.Normalize();

                float signedAngle = Math::Atan2(
                    Vector3::Cross(f2D, t2D).y,
                    Vector3::Dot(f2D, t2D)
                );

                // 最終的な回転角
                float yaw = Math::Clamp(signedAngle, -angle, angle);

                Quaternion rot = mOwnerActor->GetRotation();
                Quaternion inc(Vector3::UnitY, yaw);
                rot = Quaternion::Concatenate(rot, inc);
                mOwnerActor->SetRotation(rot);
            }
            /*
            // 距離が一定以上なら前進
            if (dist > mFollowDistance)
            {
                Vector3 pos = mOwnerActor->GetPosition();
                pos += mOwnerActor->GetForward() * mFollowSpeed * deltaTime;
                mOwnerActor->SetPosition(pos);
            }
             */
            // 壁回避付きの移動
            if (dist > mFollowDistance)
            {
                Vector3 moveDir = mOwnerActor->GetForward();
                moveDir.y = 0.0f;
                moveDir.Normalize();
                TryMoveWithRayCheck(moveDir * (-mFollowSpeed), deltaTime);
            }
        }
    }
}
