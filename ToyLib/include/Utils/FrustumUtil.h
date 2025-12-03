#pragma once

#include "Utils/MathUtil.h"
#include "Utils/Polygon.h"
#include "Frustum.h"

// ToyLib の Matrix4 に合わせたアクセス
inline Frustum BuildFrustumFromMatrix(const Matrix4& vp)
{
    Frustum fr;

    auto m = [&](int row, int col) -> float
    {
        return vp.mat[row][col];
    };

    // 左平面
    fr.planes[0].normal.x = m(0,3) + m(0,0);
    fr.planes[0].normal.y = m(1,3) + m(1,0);
    fr.planes[0].normal.z = m(2,3) + m(2,0);
    fr.planes[0].d        = m(3,3) + m(3,0);

    // 右平面
    fr.planes[1].normal.x = m(0,3) - m(0,0);
    fr.planes[1].normal.y = m(1,3) - m(1,0);
    fr.planes[1].normal.z = m(2,3) - m(2,0);
    fr.planes[1].d        = m(3,3) - m(3,0);

    // 下平面
    fr.planes[2].normal.x = m(0,3) + m(0,1);
    fr.planes[2].normal.y = m(1,3) + m(1,1);
    fr.planes[2].normal.z = m(2,3) + m(2,1);
    fr.planes[2].d        = m(3,3) + m(3,1);

    // 上平面
    fr.planes[3].normal.x = m(0,3) - m(0,1);
    fr.planes[3].normal.y = m(1,3) - m(1,1);
    fr.planes[3].normal.z = m(2,3) - m(2,1);
    fr.planes[3].d        = m(3,3) - m(3,1);

    // ニア平面
    fr.planes[4].normal.x = m(0,3) + m(0,2);
    fr.planes[4].normal.y = m(1,3) + m(1,2);
    fr.planes[4].normal.z = m(2,3) + m(2,2);
    fr.planes[4].d        = m(3,3) + m(3,2);

    // ファー平面
    fr.planes[5].normal.x = m(0,3) - m(0,2);
    fr.planes[5].normal.y = m(1,3) - m(1,2);
    fr.planes[5].normal.z = m(2,3) - m(2,2);
    fr.planes[5].d        = m(3,3) - m(3,2);

    // 正規化
    for (int i = 0; i < 6; ++i)
    {
        auto& plane = fr.planes[i];

        float len = plane.normal.Length();
        if (len > Math::NearZeroEpsilon)
        {
            plane.normal.Normalize(); // 長さ1になる
            plane.d /= len;           // 平面式を保つために d だけ手動でスケール
        }
    }

    return fr;
}

inline bool FrustumIntersectsAABB(const Frustum& fr, const Cube& box)
{
    // AABB の 8 点を生成
    Vector3 corners[8] =
    {
        Vector3(box.min.x, box.min.y, box.min.z),
        Vector3(box.max.x, box.min.y, box.min.z),
        Vector3(box.min.x, box.max.y, box.min.z),
        Vector3(box.max.x, box.max.y, box.min.z),
        Vector3(box.min.x, box.min.y, box.max.z),
        Vector3(box.max.x, box.min.y, box.max.z),
        Vector3(box.min.x, box.max.y, box.max.z),
        Vector3(box.max.x, box.max.y, box.max.z),
    };

    // 各平面で「全て裏」なら OUT
    for (int i = 0; i < 6; ++i)
    {
        const Plane& p = fr.planes[i];
        bool anyInside = false;

        for (int c = 0; c < 8; ++c)
        {
            if (p.Distance(corners[c]) >= 0.0f)
            {
                anyInside = true;
                break;
            }
        }

        if (!anyInside)
        {
            return false; // 完全に除外
        }
    }

    return true;
}
