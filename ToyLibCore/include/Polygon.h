#pragma once

#include "MathUtils.h"

// ポリゴンデータの格納用（三角形）
struct Polygon
{
    Vector3 a;
    Vector3 b;
    Vector3 c;
    
    Vector3 offsetA;
    Vector3 offsetB;
    Vector3 offsetC;
    
    void ComputeWorldTransform(Matrix4 transform);
    
};


// BOXデータの格納（Boudingboxで使う）
struct Cube
{
    Vector3 min;
    Vector3 max;
    
};


// レイ
struct Ray
{
    Vector3 start;  // 始点
    Vector3 dir;    // 正規化された方向ベクトル

    Ray() {}
    Ray(const Vector3& s, const Vector3& d)
        : start(s)
        , dir(Vector3::Normalize(d))
    {}
};

// Ray のヒット結果
struct RaycastHit
{
    bool hit = false;
    Vector3 point = Vector3::Zero;
    float distance = 0.0f;
    class Actor* actor = nullptr;
};

inline bool IntersectRayTriangle(
    const Ray& ray,
    const Vector3& v0,
    const Vector3& v1,
    const Vector3& v2,
    float& outT)
{
    const float epsilon = 1e-5f;

    Vector3 edge1 = v1 - v0;
    Vector3 edge2 = v2 - v0;

    Vector3 h = Vector3::Cross(ray.dir, edge2);
    float a = Vector3::Dot(edge1, h);

    if (fabs(a) < epsilon)
        return false;

    float f = 1.0f / a;
    Vector3 s = ray.start - v0;
    float u = f * Vector3::Dot(s, h);
    if (u < 0.0f || u > 1.0f)
        return false;

    Vector3 q = Vector3::Cross(s, edge1);
    float v = f * Vector3::Dot(ray.dir, q);
    if (v < 0.0f || u + v > 1.0f)
        return false;

    float t = f * Vector3::Dot(edge2, q);
    if (t > epsilon)
    {
        outT = t;
        return true;
    }

    return false;
}
