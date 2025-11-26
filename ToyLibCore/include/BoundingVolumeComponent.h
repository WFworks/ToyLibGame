#pragma once

#include "Component.h"
#include "MathUtils.h"

#include <vector>
#include <memory>

extern const int NUM_VERTEX;

// OBB計算用のStruct
struct OBB
{
    Vector3 pos;    // 中心座標
    Vector3 radius; // 半径
    Vector3 rot;    // 回転角度
    Vector3 axisX;  // 分離軸X
    Vector3 axisY;  // 分離軸Y
    Vector3 axisZ;  // 分離軸Z

    Vector3 min;
    Vector3 max;
    
    void operator = (const OBB& src)
    {
        OBB::min = src.min;
        OBB::max = src.max;
        OBB::pos = src.pos;
        OBB::radius = src.radius;
        OBB::rot = src.rot;
        OBB::axisX = src.axisX;
        OBB::axisY = src.axisY;
        OBB::axisZ = src.axisZ;
    }
};


// バウンディングボリューム管理
class BoundingVolumeComponent : public Component
{
public:
    // コンストラクタ
    BoundingVolumeComponent(class Actor* a);
    // デストラクタ
    ~BoundingVolumeComponent();
    // VertexArrayからBaoundingボリューム生成
    void ComputeBoundingVolume(const std::vector<std::shared_ptr<class VertexArray>> va);
    // Min Maxを直接設定
    void ComputeBoundingVolume(const Vector3 min, const Vector3 max);
    
    // バウンディングボックスを手動調整するヘルパー
    void AdjustBoundingBox(const Vector3 pos, const Vector3 sc);
    // デバッグ用
    void CreateVArray();
    
    // 座標系のUpdate
    void OnUpdateWorldTransform() override;
    
    
    // バウンディングボックス取得
    std::shared_ptr<struct Cube>  GetAABB() const { return mBoundingBox; }
    std::shared_ptr<struct OBB> GetOBB() const { return mObb; }
    // ワールド座標のAABBを返す
    struct Cube GetWorldAABB() const;
    
    std::shared_ptr<struct Polygon[]> GetPolygons() const { return mPolygons; }

    float GetRadius() const { return mRadius; }
    void SetRadius(float f) { mRadius = f; }

private:

    
    std::shared_ptr<struct Polygon[]> mPolygons;
    void CreatePolygons();

    std::shared_ptr<struct Cube> mBoundingBox;
    std::shared_ptr<struct OBB> mObb;
    float mRadius;
    
    std::unique_ptr<class WireframeComponent> mWireframe;
    
};

