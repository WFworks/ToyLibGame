#pragma once

#include "MathUtils.h"
#include <vector>
#include <string>
#include <memory>

// アクター管理
class Actor
{
public:
    enum State
    { // アクターの状態
        EActive,
        EPaused,
        EDead
    };

    // コンストラクタ
    Actor(class Application* a);
    // デストラクタ
    virtual ~Actor();

    // Updateメソッド、Applicationから呼ばれる
    void Update(float deltaTime);
    // コンポーネントの更新
    void UpdateComponents(float deltaTime);
    // Override前提 派生先の処理を書く
    virtual void UpdateActor(float deltaTime) { };

    // 入力処理 Applicationから呼ばれる Override不可
    void ProcessInput(const struct InputState& state);
    // 派生先での入力処理 Override
    virtual void ActorInput(const struct InputState& state);

    // ポジションを操作
    const Vector3& GetPosition() const { return mPosition; }
    void SetPosition(const Vector3& pos) { mPosition = pos; mIsRecomputeWorldTransform = true; }

    // スケールを操作
    float GetScale() const { return mScale; }
    void SetScale(float sc) { mScale = sc;  mIsRecomputeWorldTransform = true; }
    // 回転角を操作
	const Quaternion& GetRotation() const { return mRotation; }
	void SetRotation(const Quaternion& rot) { mRotation = rot;  mIsRecomputeWorldTransform = true; }
	
    // 座標系を更新
    void ComputeWorldTransform();
    // ワールドマトリックスを取得
    const Matrix4 GetWorldTransform() const { return mWorldTransform; }
    void SetWorldTransform(const Matrix4 mat) { mWorldTransform = mat; }
 
    // 前方を取得（Z方向）
    virtual Vector3 GetForward() { return Vector3::Transform(Vector3::UnitZ, mRotation); }
    // 右方を取得（X方向）
    virtual Vector3 GetRight() { return Vector3::Transform(Vector3::UnitX, mRotation);}
    // 上方向を取得（Y）
    virtual Vector3 GetUpward() { return Vector3::Transform(Vector3::UnitY, mRotation); }
    
    // 前方向（Z方向）をベクトルに合わせる
    void SetForward(const Vector3 dir);
    
    // ステータスを操作
	State GetState() const { return mStatus; }
	void SetState(State state) { mStatus = state; }

    // オーナーのアプリケーションポインタを取る
    class Application* GetApp() { return mApp; }


	// コンポーネントの追加削除
	//void AddComponent(class Component* component);
	void AddComponent(std::unique_ptr<class Component> component);
    void RemoveComponent(class Component* component);

    // コンポーネントを生成
    template <typename T, typename... Args>
    T* CreateComponent(Args&&... args)
    {
        auto comp = std::make_unique<T>(this, std::forward<Args>(args)...);
        T* rawPtr = comp.get();
        AddComponent(std::unique_ptr<class Component>(std::move(comp)));  // ★ここでComponent型に変換
        return rawPtr;
    }

    // 該当するコンポーネントを返す（１つ）
    template <typename T>
    T* GetComponent() const
    {
        for (const auto& comp : mComponents)
        {
            // dynamic_castで型チェック（RTTI必要）
            if (auto casted = dynamic_cast<T*>(comp.get()))
            {
                return casted;
            }
        }
        return nullptr;
    }
    // 該当するコンポーネントを全てVectorで返す
    template <typename T>
    std::vector<T*> GetAllComponents() const
    {
        std::vector<T*> results;
        for (auto& comp : mComponents)
        {
            T* casted = dynamic_cast<T*>(comp.get());
            if (casted)
            {
                results.emplace_back(casted);
            }
        }
        return results;
    }

    // Actor IDを設定/取得
    void SetActorID(const std::string actorID) { mActorID = actorID; }
    std::string GetActorID() const { return mActorID; }
    
    
private:
    // マトリックス
    Matrix4     mWorldTransform;
    Vector3     mPosition;
    Quaternion  mRotation;
    float       mScale;
    // turueの時のみマトリックスを再計算
    bool        mIsRecomputeWorldTransform;

    // 保有コンポーネント
    std::vector<std::unique_ptr<class Component>> mComponents;
    // アプリクラス
    class Application* mApp;
    
    // ステータス
    enum State mStatus; // emum State 状態を管理
    
    // actor ID
    std::string mActorID;

    

};
