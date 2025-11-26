#include "PhysWorld.h"
#include "Application.h"
#include "VertexArray.h"
#include "Actor.h"
#include "CameraComponent.h"
#include "BoundingVolumeComponent.h"
#include "Polygon.h"
#include "ColliderComponent.h"
#include "MoveComponent.h"
#include "MathUtils.h"
#include <algorithm>


PhysWorld::PhysWorld()
{
}

PhysWorld::~PhysWorld()
{
}

void PhysWorld::Test()
{
    
    for (auto c : mColliders)
    {
        c->ClearCollidBuffer();
    }
    
    CollideAndCallback(C_PLAYER, C_ENEMY);                       // ヒットのみ
    CollideAndCallback(C_PLAYER, C_BULLET);                      // ヒットのみ
    //CollideAndCallback(C_PLAYER, C_WALL, true, false);          // 壁ずり（Y方向除外）
    CollideAndCallback(C_ENEMY, C_WALL, true, false);           // 敵の壁押し戻し
    // Laser vs Enemy（Ray vs Mesh）
    for (auto c1 : mColliders)
    {
        if (!c1->HasFlag(C_LASER)) continue;
        if (!c1->GetDisp()) continue;

        Ray ray = c1->GetRay();  // Laserが保持するRay（要実装）

        for (auto c2 : mColliders)
        {
            if (c1 == c2) continue;
            if (!c2->HasFlag(C_ENEMY)) continue;
            if (!c2->GetDisp()) continue;

            const auto& polygons = c2->GetBoundingVolume()->GetPolygons(); // Polygon配列
            bool hit = false;
            float closestT = Math::Infinity;
            Vector3 hitPoint;

            for (int i = 0; i < NUM_VERTEX; i++) // NUM_VERTEX = polygon数
            {
                const auto& poly = polygons[i];
                float t;
                if (IntersectRayTriangle(ray, poly.a, poly.b, poly.c, t))
                {
                    if (t < closestT)
                    {
                        closestT = t;
                        hit = true;
                        hitPoint = ray.start + ray.dir * t;
                    }
                }
            }

            if (hit)
            {
                // ヒットしたらコールバック発火
                c1->Collided(c2);
                c2->Collided(c1);
                // デバッグ表示などにhitPointを使ってもOK
            }
        }
    }

}

// OBBの投影距離比較
bool PhysWorld::CompareLengthOBB(const OBB* cA, const OBB* cB, const Vector3 vSep, const Vector3 vDistance)
{
    // 分離軸上のAからBの距離
    float length = fabsf(Vector3::Dot(vSep, vDistance));


    // 分離軸上でAから最も遠いAの頂点までの距離
    float lenA =
          fabs(Vector3::Dot(cA->axisX, vSep) * cA->radius.x)
            + fabs(Vector3::Dot(cA->axisY, vSep) * cA->radius.y)
            + fabs(Vector3::Dot(cA->axisZ, vSep) * cA->radius.z);

    // 分離軸上でBから最も遠いBの頂点までの距離
    float lenB =
          fabs(Vector3::Dot(cB->axisX, vSep) * cB->radius.x)
          + fabs(Vector3::Dot(cB->axisY, vSep) * cB->radius.y)
          + fabs(Vector3::Dot(cB->axisZ, vSep) * cB->radius.z);

    // 距離による判定
    if(length > lenA + lenB)
    {
        return false;
    }
    return true;
}




// 回転に対応したOBBCollid
bool PhysWorld::JudgeWithOBB(ColliderComponent* col1, ColliderComponent* col2)
{
    
    auto obb1 = col1->GetBoundingVolume()->GetOBB();
    auto obb2 = col2->GetBoundingVolume()->GetOBB();
    
    
    // 判定する
    return  IsCollideBoxOBB(obb1.get(), obb2.get());
}

// 衝突しているかどうか
bool PhysWorld::IsCollideBoxOBB(const OBB* cA, const OBB* cB)
{
    // 中心間の距離ベクトル算出
    Vector3 vDistance = cB->pos - cA->pos;


    // 分離軸を比較
    if(!CompareLengthOBB(cA, cB, cA->axisX, vDistance)) return false;
    if(!CompareLengthOBB(cA, cB, cA->axisY, vDistance)) return false;
    if(!CompareLengthOBB(cA, cB, cA->axisZ, vDistance)) return false;
    if(!CompareLengthOBB(cA, cB, cB->axisX, vDistance)) return false;
    if(!CompareLengthOBB(cA, cB, cB->axisY, vDistance)) return false;
    if(!CompareLengthOBB(cA, cB, cB->axisZ, vDistance)) return false;

    // 分離軸同士の外積を比較
    Vector3 vSep = Vector3::Cross(cA->axisX, cB->axisX);
    if(!CompareLengthOBB(cA, cB, vSep, vDistance)) return false;
    
    vSep = Vector3::Cross(cA->axisX, cB->axisY);
    if(!CompareLengthOBB(cA, cB, vSep, vDistance)) return false;
    
    vSep = Vector3::Cross(cA->axisX, cB->axisZ);
    if(!CompareLengthOBB(cA, cB, vSep, vDistance)) return false;
    
    vSep = Vector3::Cross(cA->axisY, cB->axisX);
    if(!CompareLengthOBB(cA, cB, vSep, vDistance)) return false;
    
    vSep = Vector3::Cross(cA->axisY, cB->axisY);
    if(!CompareLengthOBB(cA, cB, vSep, vDistance)) return false;
    
    vSep = Vector3::Cross(cA->axisY, cB->axisZ);
    if(!CompareLengthOBB(cA, cB, vSep, vDistance)) return false;
    
    vSep = Vector3::Cross(cA->axisZ, cB->axisX);
    if(!CompareLengthOBB(cA, cB, vSep, vDistance)) return false;
    
    vSep = Vector3::Cross(cA->axisZ, cB->axisY);
    if(!CompareLengthOBB(cA, cB, vSep, vDistance)) return false;
    
    vSep = Vector3::Cross(cA->axisZ, cB->axisZ);
    if(!CompareLengthOBB(cA, cB, vSep, vDistance)) return false;

    return true;
}

// バウンディングスフィアの距離で判定
bool PhysWorld::JudgeWithRadius(class ColliderComponent* col1, class ColliderComponent* col2)
{
    auto ditance = col1->GetPosition() - col2->GetPosition();
    float len = ditance.Length();
    float thleshold = col1->GetBoundingVolume()->GetRadius() + col2->GetBoundingVolume()->GetRadius();

    
    if(thleshold > len)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void PhysWorld::AddCollider(ColliderComponent* c)
{
    mColliders.emplace_back(c);
}
void PhysWorld::RemoveCollider(ColliderComponent* c)
{
    auto iter = std::find(mColliders.begin(), mColliders.end(), c);
    if (iter != mColliders.end())
    {
        mColliders.erase(iter);
    }
}


// XZ平面に投影し、点がポリゴン内に存在すればTrue
bool PhysWorld::IsInPolygon(const Polygon* pl, const Vector3 p) const
{

    if(((pl->b.x - pl->a.x) * (p.z - pl->a.z) -
        (pl->b.z - pl->a.z) * (p.x - pl->a.x)) < 0)
    {
            return false;
    }
    if(((pl->c.x - pl->b.x) * (p.z - pl->b.z) -
        (pl->c.z - pl->b.z) * (p.x - pl->b.x)) < 0)
    {
            return false;
    }
    if(((pl->a.x - pl->c.x) * (p.z - pl->c.z) -
        (pl->a.z - pl->c.z) * (p.x - pl->c.x)) < 0)
    {
            return false;
    }

    return true;

}


// ポリゴン内のある座標における高さを返す
float PhysWorld::PolygonHeight(const Polygon* pl, const Vector3 p) const
{

    float wa, wb, wc;    // 平明方程式の係数

    wa = (pl->c.z - pl->a.z) * (pl->b.y - pl->a.y) -
         (pl->c.y - pl->a.y) * (pl->b.z - pl->a.z);
    wb = (pl->c.y - pl->a.y) * (pl->b.x - pl->a.x) -
         (pl->c.x - pl->a.x) * (pl->b.y - pl->a.y);
    wc = (pl->c.x - pl->a.x) * (pl->b.z - pl->a.z) -
         (pl->c.z - pl->a.z) * (pl->b.x - pl->a.x);


    return -(wa * (p.x - pl->a.x) + wb * (p.z - pl->a.z)) / wc + pl->a.y;
}

bool PhysWorld::CompareLengthOBB_MTV(const OBB* cA, const OBB* cB, const Vector3& vSep, const Vector3& vDistance, MTVResult& mtv)
{
    if (vSep.LengthSq() < 1e-6f) return true; // 無効軸はスキップ

    float length = fabsf(Vector3::Dot(vSep, vDistance));

    float lenA =
        fabs(Vector3::Dot(cA->axisX, vSep) * cA->radius.x) +
        fabs(Vector3::Dot(cA->axisY, vSep) * cA->radius.y) +
        fabs(Vector3::Dot(cA->axisZ, vSep) * cA->radius.z);

    float lenB =
        fabs(Vector3::Dot(cB->axisX, vSep) * cB->radius.x) +
        fabs(Vector3::Dot(cB->axisY, vSep) * cB->radius.y) +
        fabs(Vector3::Dot(cB->axisZ, vSep) * cB->radius.z);

    float overlap = lenA + lenB - length;

    if (overlap < 0.0f)
    {
        return false;
    }

    if (overlap < mtv.depth)
    {
        mtv.depth = overlap;
        mtv.axis = vSep;
        mtv.valid = true;
    }

    return true;
}

bool PhysWorld::IsCollideBoxOBB_MTV(const OBB* cA, const OBB* cB, MTVResult& mtv)
{
    Vector3 vDistance = cB->pos - cA->pos;

    return
        CompareLengthOBB_MTV(cA, cB, cA->axisX, vDistance, mtv) &&
        CompareLengthOBB_MTV(cA, cB, cA->axisY, vDistance, mtv) &&
        CompareLengthOBB_MTV(cA, cB, cA->axisZ, vDistance, mtv) &&
        CompareLengthOBB_MTV(cA, cB, cB->axisX, vDistance, mtv) &&
        CompareLengthOBB_MTV(cA, cB, cB->axisY, vDistance, mtv) &&
        CompareLengthOBB_MTV(cA, cB, cB->axisZ, vDistance, mtv) &&

        CompareLengthOBB_MTV(cA, cB, Vector3::Cross(cA->axisX, cB->axisX), vDistance, mtv) &&
        CompareLengthOBB_MTV(cA, cB, Vector3::Cross(cA->axisX, cB->axisY), vDistance, mtv) &&
        CompareLengthOBB_MTV(cA, cB, Vector3::Cross(cA->axisX, cB->axisZ), vDistance, mtv) &&
        CompareLengthOBB_MTV(cA, cB, Vector3::Cross(cA->axisY, cB->axisX), vDistance, mtv) &&
        CompareLengthOBB_MTV(cA, cB, Vector3::Cross(cA->axisY, cB->axisY), vDistance, mtv) &&
        CompareLengthOBB_MTV(cA, cB, Vector3::Cross(cA->axisY, cB->axisZ), vDistance, mtv) &&
        CompareLengthOBB_MTV(cA, cB, Vector3::Cross(cA->axisZ, cB->axisX), vDistance, mtv) &&
        CompareLengthOBB_MTV(cA, cB, Vector3::Cross(cA->axisZ, cB->axisY), vDistance, mtv) &&
        CompareLengthOBB_MTV(cA, cB, Vector3::Cross(cA->axisZ, cB->axisZ), vDistance, mtv);
}

Vector3 PhysWorld::ComputePushBackDirection(ColliderComponent* a, ColliderComponent* b, bool allowY)
{
    MTVResult mtv;

    auto obb1 = a->GetBoundingVolume()->GetOBB();
    auto obb2 = b->GetBoundingVolume()->GetOBB();

    if (!IsCollideBoxOBB_MTV(obb1.get(), obb2.get(), mtv) || !mtv.valid)
    {
        // fallback
        Vector3 delta = a->GetPosition() - b->GetPosition();
        if (!allowY) delta.y = 0.0f;
        if (delta.LengthSq() > Math::NearZeroEpsilon)
        {
            delta.Normalize();
        }
        else
        {
            delta = Vector3::UnitZ;
        }
        return delta * 0.1f;
        //return delta * 0.05f;
    }

    // MTVに基づく押し戻し（Y方向を除外して壁ずり対応）
    Vector3 pushAxis = mtv.axis;
    if (!allowY) pushAxis.y = 0.0f;

    // MTVの向きを修正：a→bの方向でなければ反転
    Vector3 dirAB = a->GetPosition() - b->GetPosition();
    if (Vector3::Dot(pushAxis, dirAB) < 0.0f)
    {
        pushAxis *= -1.0f;
    }

    if (pushAxis.LengthSq() > Math::NearZeroEpsilon)
    {
        pushAxis.Normalize();
        return pushAxis * (mtv.depth + 0.05f);
    }

    return Vector3::Zero;
}

bool PhysWorld::GetNearestGroundY(const Actor* a, float& outY) const
{
    if (!a) return false;
    //if (velocity > 0) return false;

    const auto* foot = FindFootCollider(a);
    if (!foot) return false;

    const Cube box = foot->GetBoundingVolume()->GetWorldAABB();


    float highest = -FLT_MAX;
    bool found = false;
    
    float footY = box.min.y;

    // C_GROUND コライダーから、一番高い地面を探す
    for (auto c : mColliders)
    {
        if (!c->HasFlag(C_GROUND)) continue;
        if (c->GetOwner() == a) continue;

        const Cube other = c->GetBoundingVolume()->GetWorldAABB();
        
        // XZ平面上の重なりをチェック
        const bool xzOverlap =
            box.max.x > other.min.x && box.min.x < other.max.x &&
            box.max.z > other.min.z && box.min.z < other.max.z;

        const float yGap = footY - other.max.y;

        // 自分の足より低くて、これまでのColliderより高ければ記録
        if (xzOverlap && yGap > 0.0f && highest < other.max.y)
        {
            highest = other.max.y;
            found = true;
        }
    }

    // --- 地形ポリゴンによる接地判定（オプション） ---
    const Vector3 center = a->GetPosition();
    const float terrainY = GetGroundHeightAt(center);
    if (terrainY > highest)
    {
        highest = terrainY;
        found = true;
    }

    if (found) outY = highest;
    return found;
}

float PhysWorld::GetGroundHeightAt(const Vector3& pos) const
{
    float highestY = -FLT_MAX;
    for (const auto& poly : mTerrainPolygons)
    {
        if (IsInPolygon(&poly, pos))
        {
            float y = PolygonHeight(&poly, pos);
            if (y > highestY)
            {
                highestY = y;
            }
        }
    }
    return highestY;
}

void PhysWorld::SetGroundPolygons(const std::vector<Polygon>& polys)
{
    mTerrainPolygons = polys;
}

void PhysWorld::CollideAndCallback(uint32_t flagA,
                                   uint32_t flagB,
                                   bool doPushBack,
                                   bool allowY,
                                   bool stopVerticalSpeed)
{
    for (auto* c1 : mColliders)
    {
        if (!c1->GetDisp() || !c1->HasFlag(flagA)) continue;

        Vector3 totalPush = Vector3::Zero;
        bool collided = false;

        for (auto* c2 : mColliders)
        {
            if (!c2->GetDisp() || !c2->HasFlag(flagB)) continue;
            if (c1->GetOwner() == c2->GetOwner()) continue;

            if (JudgeWithRadius(c1, c2) && JudgeWithOBB(c1, c2))
            {
                c1->Collided(c2);
                c2->Collided(c1);
                collided = true;

                if (doPushBack)
                {
                    Vector3 push = ComputePushBackDirection(c1, c2, allowY);
                    totalPush += push;
                }
            }
        }

        if (collided && doPushBack)
        {
            Vector3 newPos = c1->GetOwner()->GetPosition() + totalPush;
            c1->GetOwner()->SetPosition(newPos);

            if (stopVerticalSpeed)
            {
                if (auto* move = c1->GetOwner()->GetComponent<MoveComponent>())
                {
                    move->SetVerticalSpeed(0.0f);
                }
            }
        }
    }
}

 
ColliderComponent* PhysWorld::FindFootCollider(const Actor* a) const
{
    for (auto* comp : a->GetAllComponents<ColliderComponent>())
    {
        if (comp->HasFlag(C_FOOT))
            return comp;
    }
    return nullptr;
}



bool PhysWorld::IntersectRayOBB(const Ray& ray, const OBB* obb, float& outT) const
{
    const float epsilon = 1e-6f;
    Vector3 p = obb->pos - ray.start;
    float tMin = 0.0f;
    float tMax = Math::Infinity;

    for (int i = 0; i < 3; ++i)
    {
        Vector3 axis;
        float r = 0.0f;

        if (i == 0) { axis = obb->axisX; r = obb->radius.x; }
        if (i == 1) { axis = obb->axisY; r = obb->radius.y; }
        if (i == 2) { axis = obb->axisZ; r = obb->radius.z; }

        float e = Vector3::Dot(axis, p);
        float f = Vector3::Dot(ray.dir, axis);

        if (fabsf(f) > epsilon)
        {
            float t1 = (e + r) / f;
            float t2 = (e - r) / f;
            if (t1 > t2) std::swap(t1, t2);

            tMin = std::max(tMin, t1);
            tMax = std::min(tMax, t2);

            if (tMin > tMax)
                return false;
        }
        else
        {
            if (-e - r > 0.0f || -e + r < 0.0f)
                return false;
        }
    }

    outT = tMin;
    return true;
}

bool PhysWorld::RayHitWall(const Vector3& start, const Vector3& end, Vector3& hitPos) const
{
    Ray ray;
    ray.start = start;
    ray.dir = end - start;
    float rayLen = ray.dir.Length();
    if (rayLen < Math::NearZeroEpsilon) return false;
    ray.dir.Normalize();

    float closestT = rayLen;
    bool hit = false;

    for (auto* col : mColliders)
    {
        if (!col->HasFlag(C_WALL)) continue;
        auto obb = col->GetBoundingVolume()->GetOBB();

        float t;
        if (IntersectRayOBB(ray, obb.get(), t))
        {
            if (t < closestT)
            {
                closestT = t;
                hit = true;
            }
        }
    }

    if (hit)
    {
        hitPos = ray.start + ray.dir * (closestT - 0.01f); // ほんの少し手前で止める
        return true;
    }

    return false;
}
