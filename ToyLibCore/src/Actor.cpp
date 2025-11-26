#include "Application.h"
#include "Actor.h"
#include "Component.h"

#include <algorithm>
#include <iostream>
#include <memory>

//コンストラクタ
Actor::Actor(Application* a)
: mStatus(EActive)
, mPosition(Vector3::Zero)
, mRotation(Quaternion::Identity)
, mScale(1.0f)
, mApp(a)
, mIsRecomputeWorldTransform(true)
, mActorID("Unnamed Actor")
{

}

// デストラクタ
Actor::~Actor()
{
   

}

// メインルーチン
void Actor::Update(float deltaTime)
{
    // EActiveの場合にComponentもUpdate
    if (mStatus == EActive)
    {
        // 派生先の処理を呼ぶ
        UpdateActor(deltaTime);
        // コンポーネントを更新
        UpdateComponents(deltaTime);
        // 座標系更新
        ComputeWorldTransform();
    }
}

// コンポーネントのUpdateを呼ぶ
void Actor::UpdateComponents(float deltaTime)
{
    for (auto& comp : mComponents)
    {
        comp->Update(deltaTime);
    }
}


// 入力を受け取る
void Actor::ProcessInput(const struct InputState& state)
{
    if (mStatus == EActive)
    {
        // コンポーネントの入力処理
        for (auto& comp : mComponents)
        {
            comp->ProcessInput(state);
        }
        // 派生先の入力処理に渡す
        ActorInput(state);
    }
}

// 入力処理　(実装は派生先)
void Actor::ActorInput(const struct InputState& state)
{
}

// ワールドマトリックス
void Actor::ComputeWorldTransform()
{
    if (mIsRecomputeWorldTransform)
    {
        mIsRecomputeWorldTransform = false;
        // Scale → rotate → translate
        mWorldTransform = Matrix4::CreateScale(mScale);
        mWorldTransform *= Matrix4::CreateFromQuaternion(mRotation);
        mWorldTransform *= Matrix4::CreateTranslation(mPosition);


        // 各Componentも計算する
        for (auto& comp : mComponents)
        {
            comp->OnUpdateWorldTransform();
        }
    }
}

// コンポーネントを追加
void Actor::AddComponent(std::unique_ptr<Component> component)
{
    int order = component->GetUpdateOrder();
    auto iter = mComponents.begin();
    for (; iter != mComponents.end(); ++iter)
    {
        if (order < (*iter)->GetUpdateOrder())
        {
            break;
        }
    }
    mComponents.insert(iter, std::move(component));
}
void Actor::RemoveComponent(Component* component)
{
    auto iter = std::find_if(mComponents.begin(), mComponents.end(),
        [component](const std::unique_ptr<Component>& c) { return c.get() == component; });

    if (iter != mComponents.end())
    {
        mComponents.erase(iter);
    }
}

void Actor::SetForward(const Vector3 dir)
{
    // Y成分を無視（XZ平面に投影）
    Vector3 flatDir = Vector3(dir.x, 0.0f, dir.z);

    if (flatDir.LengthSq() == 0.0f)
        return; // 方向なし

    flatDir = Vector3::Normalize(flatDir);

    // atan2(x, z) で Yaw を取得（Zが前提、Xが右）
    float yaw = std::atan2(flatDir.x, flatDir.z);

    // YawからQuaternion生成（Pitch = 0, Roll = 0）
    Quaternion rot = Quaternion::FromEulerDegrees(Vector3(0.0f, yaw, 0.f));

    SetRotation(rot);
}
