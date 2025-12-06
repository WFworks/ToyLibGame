#pragma once

#include "Utils/MathUtil.h"
#include "Asset/Geometry/Polygon.h"
#include "Frustum.h"

//------------------------------------------------------------------------------
// BuildFrustumFromMatrix
//------------------------------------------------------------------------------
// ・View * Projection 行列（VP）から視錐台（Frustum）を生成するヘルパー。
// ・行列は ToyLib の Matrix4（列優先 / mat[row][col]）前提。
// ・ニア/ファー、上下左右の 6 平面を切り出し、正規化して返す。
// ・カメラの VP 行列からそのまま Frustum を作りたいときに使用。
//------------------------------------------------------------------------------
inline Frustum BuildFrustumFromMatrix(const Matrix4& vp)
{
    Frustum fr;

    // ToyLib の Matrix4 に合わせたアクセス
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

    // 各平面を正規化（normal の長さを 1 に）
    for (int i = 0; i < 6; ++i)
    {
        auto& plane = fr.planes[i];

        float len = plane.normal.Length();
        if (len > Math::NearZeroEpsilon)
        {
            plane.normal.Normalize(); // 長さ 1
            plane.d /= len;           // 平面式を維持するよう d をスケール
        }
    }

    return fr;
}

//------------------------------------------------------------------------------
// FrustumIntersectsAABB
//------------------------------------------------------------------------------
// ・視錐台と AABB（Cube）の簡易交差判定。
// ・AABB の 8 頂点のうち、各平面で「1点でも表側にあれば」その平面では生存。
// ・どこか 1 つの平面で「8点すべてが裏側」→ 完全に外側とみなして false を返す。
// ・true が返っても完全に「中にある」保証はなく、
//   「視錐台と重なっている可能性が高い」というラフな判定（カリング向け）。
//------------------------------------------------------------------------------
inline bool FrustumIntersectsAABB(const Frustum& fr, const toy::Cube& box)
{
    // AABB の 8 頂点を生成
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

    // 各平面ごとにチェック：
    // 「全頂点が平面の裏側（Distance < 0）」であれば OUT
    for (int i = 0; i < 6; ++i)
    {
        const Plane& p = fr.planes[i];
        bool anyInside = false;

        for (int c = 0; c < 8; ++c)
        {
            if (p.Distance(corners[c]) >= 0.0f)
            {
                // 1点でも表側 or 境界上にあれば、この平面ではまだ「生存」
                anyInside = true;
                break;
            }
        }

        // 1点も表側に出てこなかった → 完全に外側
        if (!anyInside)
        {
            return false; // 視錐台の外
        }
    }

    // 全平面で「完全に外」にならなかった → 交差しているとみなす
    return true;
}
