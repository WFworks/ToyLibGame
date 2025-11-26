// ハイブリッド方式の GetNearestGroundY に対応した PhysWorld
#pragma once

#include "MathUtils.h"
#include "ColliderComponent.h"
#include <vector>

struct MTVResult
{
    Vector3 axis = Vector3::Zero;
    float depth = Math::Infinity;
    bool valid = false;
};

// 物理計算系のクラス
class PhysWorld
{
public:
    PhysWorld();
    ~PhysWorld();

    void ComputeGroundHeight();
    void Test();

    // コライダー管理
    void AddCollider(class ColliderComponent* c);
    void RemoveCollider(class ColliderComponent* c);

    // 地面情報インターフェイス
    float GetGroundHeightAt(const Vector3& pos) const;
    // 一番近い地面の高さを調べる
    bool GetNearestGroundY(const class Actor* a, float& outY) const;
    void SetGroundPolygons(const std::vector<Polygon>& polys);
    
    // RayCCB実装用
    bool RayHitWall(const Vector3& start, const Vector3& end, Vector3& hitPos) const;
    bool IntersectRayOBB(const Ray& ray, const struct OBB* obb, float& outT) const;
    
    // 判定用コールバック
    void CollideAndCallback(uint32_t flagA,
                            uint32_t flagB,
                            bool doPushBack = false,
                            bool allowY = false,
                            bool stopVerticalSpeed = false);

private:
    // 衝突判定
    bool CompareLengthOBB(const struct OBB* cA, const struct OBB* cB, const Vector3 vSep, const Vector3 vDistance);
    bool JudgeWithOBB(class ColliderComponent* col1, class ColliderComponent* col2);
    bool IsCollideBoxOBB(const OBB* cA, const OBB* cB);
    bool JudgeWithRadius(class ColliderComponent* col1, class ColliderComponent* col2);

    // 壁ずり用
    Vector3 ComputePushBackDirection(class ColliderComponent* a, class ColliderComponent* b, bool allowY);
    bool CompareLengthOBB_MTV(const OBB* cA, const OBB* cB, const Vector3& vSep, const Vector3& vDistance, MTVResult& mtv);
    bool IsCollideBoxOBB_MTV(const OBB* cA, const OBB* cB, MTVResult& mtv);

    // 地表計算
    bool IsInPolygon(const struct Polygon* pl, const struct Vector3 p) const;
    float PolygonHeight(const struct Polygon* pl, const struct Vector3 p) const;
    
    
    
    class ColliderComponent* FindFootCollider(const Actor* a) const;
    

    // コライダー保持
    std::vector<class ColliderComponent*> mColliders;

    // 地形（静的Mesh）
    std::vector<struct Polygon> mTerrainPolygons;
};
