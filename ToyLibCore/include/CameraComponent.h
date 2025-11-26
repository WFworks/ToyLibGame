#pragma once


#include "Component.h"
#include "MathUtils.h"
#include <memory>

// カメラ管理コンポーネント
class CameraComponent : public Component
{
public:
    // コンストラクタ
	CameraComponent(class Actor* owner, int updateOrder = 200);
    void Update(float deltaTime) override;

    
protected:
    // カメラの座標
    Vector3 mCameraPosition;
    // ビューマトリックスを設定
	void SetViewMatrix(const Matrix4& view);
    void SetCameraPosition(const Vector3 pos);
    
    
    // カメラの場所計算用
    std::unique_ptr<class Actor> mCameraActor;
};
