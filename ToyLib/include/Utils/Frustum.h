#pragma once

#include "Utils/MathUtil.h"

struct Plane
{
    Vector3 normal; // 法線ベクトル
    float d;        // ax + by + cz + d = 0 の d

    // 点との signed 距離
    float Distance(const Vector3& p) const
    {
        return Vector3::Dot(normal, p) + d;
    }
};

struct Frustum
{
    Plane planes[6]; // 0:Left,1:Right,2:Bottom,3:Top,4:Near,5:Far
};
