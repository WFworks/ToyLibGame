#include "BoundingVolumeComponent.h"
#include "WireframeComponent.h"
#include "Actor.h"
#include "Polygon.h"
#include "VertexArray.h"
#include "Shader.h"

#include "Application.h"
#include "Renderer.h"
#include "Texture.h"
#include "Polygon.h"

#include <vector>
#include <algorithm>

const int NUM_VERTEX = 12;

// コンストラクタ
BoundingVolumeComponent::BoundingVolumeComponent(Actor* a)
: Component(a)
, mRadius(0.0f)
{
    mBoundingBox = std::make_shared<Cube>();
    mObb = std::make_shared<OBB>();
    mPolygons.reset(new Polygon[NUM_VERTEX]);
    
    if (mOwnerActor->GetApp()->GetRenderer()->IsDebugMode())
    {
        mWireframe = std::make_unique<WireframeComponent>(mOwnerActor, 1000);
        mWireframe->SetColor(Vector3(1,0,0.5f));
    }
}

// デストラクタ
BoundingVolumeComponent::~BoundingVolumeComponent()
{

}


void BoundingVolumeComponent::OnUpdateWorldTransform()
{
    // バウンディングボックスパラメータをセット
    mObb->pos = mOwnerActor->GetPosition();
    float sc1 = mOwnerActor->GetScale();
    mObb->max = mBoundingBox->max * sc1;
    mObb->min = mBoundingBox->min * sc1;
    
    Quaternion q1 = mOwnerActor->GetRotation();
    Matrix4 mRot1 = Matrix4::CreateFromQuaternion(q1);
      
    mObb->axisX = mRot1.GetXAxis();
    mObb->axisY = mRot1.GetYAxis();
    mObb->axisZ = mRot1.GetZAxis();

    // 半径を設定
    mObb->radius = //Vector3::Dot(comp1->GetBoundingBox()->max, comp1->GetBoundingBox()->min);
              Vector3((fabsf(mObb->max.x) + fabsf(mObb->min.x)) / 2,
                        (fabsf(mObb->max.y) + fabsf(mObb->min.y)) / 2,
                        (fabsf(mObb->max.z) + fabsf(mObb->min.z)) / 2);
    

    // 角度、AXIS、ちょっと怪しいかも。。。
    mObb->rot = Vector3(mObb->axisX.x, mObb->axisY.y, mObb->axisZ.z);
    // バウンディングスフィア
    mRadius = mObb->radius.Length();
    
    
}

// バウンディングボリュームを計算
void BoundingVolumeComponent::ComputeBoundingVolume(const std::vector<std::shared_ptr<VertexArray>> va)
{
    
    // 複数VertexArrayをまとめて計算
    for (const auto& v : va)
    {
        const auto& polygons = v->GetPolygons();
        for (const auto& poly : polygons)
        {
            //const auto poly = p;

            mBoundingBox->min.x = std::min({mBoundingBox->min.x, poly.a.x, poly.b.x, poly.c.x});
            mBoundingBox->max.x = std::max({mBoundingBox->max.x, poly.a.x, poly.b.x, poly.c.x});

            mBoundingBox->min.y = std::min({mBoundingBox->min.y, poly.a.y, poly.b.y, poly.c.y});
            mBoundingBox->max.y = std::max({mBoundingBox->max.y, poly.a.y, poly.b.y, poly.c.y});

            mBoundingBox->min.z = std::min({mBoundingBox->min.z, poly.a.z, poly.b.z, poly.c.z});
            mBoundingBox->max.z = std::max({mBoundingBox->max.z, poly.a.z, poly.b.z, poly.c.z});
        }
    }
    
    CreateVArray();
    CreatePolygons();
    
}

// ポリゴンデータを生成
void BoundingVolumeComponent::CreatePolygons()
{
    
    Vector3 V0(mBoundingBox->min.x, mBoundingBox->min.y, mBoundingBox->min.z);
    Vector3 V1(mBoundingBox->max.x, mBoundingBox->min.y, mBoundingBox->min.z);
    Vector3 V2(mBoundingBox->max.x, mBoundingBox->min.y, mBoundingBox->max.z);
    Vector3 V3(mBoundingBox->min.x, mBoundingBox->min.y, mBoundingBox->max.z);
    Vector3 V4(mBoundingBox->min.x, mBoundingBox->max.y, mBoundingBox->min.z);
    Vector3 V5(mBoundingBox->max.x, mBoundingBox->max.y, mBoundingBox->min.z);
    Vector3 V6(mBoundingBox->max.x, mBoundingBox->max.y, mBoundingBox->max.z);
    Vector3 V7(mBoundingBox->min.x, mBoundingBox->max.y, mBoundingBox->max.z);

    // Z- 面（前）
    mPolygons[0] = { V0, V4, V5 };
    mPolygons[1] = { V0, V5, V1 };

    // X+ 面（右）
    mPolygons[2] = { V1, V5, V6 };
    mPolygons[3] = { V1, V6, V2 };

    // Z+ 面（背）
    mPolygons[4] = { V2, V6, V7 };
    mPolygons[5] = { V2, V7, V3 };

    // X- 面（左）
    mPolygons[6] = { V3, V7, V4 };
    mPolygons[7] = { V3, V4, V0 };

    // Y+ 面（上）
    mPolygons[8] = { V4, V7, V6 };
    mPolygons[9] = { V4, V6, V5 };

    // Y- 面（下）
    mPolygons[10] = { V3, V0, V1 };
    mPolygons[11] = { V3, V1, V2 };

}


// Min Maxを直接指定
void BoundingVolumeComponent::ComputeBoundingVolume(const Vector3 min, const Vector3 max)
{
    mBoundingBox->min = min;
    mBoundingBox->max = max;
    CreateVArray();
    CreatePolygons();
}

// バウンディングボックスを手動調整する（場所とサイズ（倍率））
void BoundingVolumeComponent::AdjustBoundingBox(const Vector3 pos, const Vector3 sc)
{
    
    mBoundingBox->max += pos;
    mBoundingBox->min += pos;
    
    mBoundingBox->max.x *= sc.x;
    mBoundingBox->min.x *= sc.x;
    mBoundingBox->max.y *= sc.y;
    mBoundingBox->min.y *= sc.y;
    mBoundingBox->max.z *= sc.z;
    mBoundingBox->min.z *= sc.z;
    CreateVArray();
    CreatePolygons();

}




// デバッグ用
void BoundingVolumeComponent::CreateVArray()
{
    // ボックス用頂点バッファ（座標、法線、UV）
    float verts[] =
    {
        mBoundingBox->min.x, mBoundingBox->min.y, mBoundingBox->min.z, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // 0
        mBoundingBox->min.x, mBoundingBox->max.y, mBoundingBox->min.z, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // 1
        mBoundingBox->max.x, mBoundingBox->min.y, mBoundingBox->min.z, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, // 2
        mBoundingBox->max.x, mBoundingBox->max.y, mBoundingBox->min.z, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // 3
        mBoundingBox->max.x, mBoundingBox->min.y, mBoundingBox->max.z, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // 4
        mBoundingBox->max.x, mBoundingBox->max.y, mBoundingBox->max.z, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // 5
        mBoundingBox->min.x, mBoundingBox->min.y, mBoundingBox->max.z, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, // 6
        mBoundingBox->min.x, mBoundingBox->max.y, mBoundingBox->max.z, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f  // 7
        
    };
    
    unsigned int index[] = {
        0, 1, 3,
        3, 2, 0,
        
        4, 2, 3,
        3, 5, 4,
        
        5, 4, 6,
        6, 7, 5,
        
        0, 6, 7,
        7, 1, 0,
        
        3, 1, 7,
        7, 5, 3,
        
        2, 4, 6,
        6, 0, 2
        

    };
    
    //mVertexArray = std::make_shared<VertexArray>(BBverts, 8, (unsigned int*)BBindex, (unsigned int)36);
    if (mWireframe)
    {
        mWireframe->SetVertexArray(std::make_shared<VertexArray>(verts, 8, (unsigned int*)index, (unsigned int)36));
    }

}

// ワールド空間でのバウンディングボックスを取得
Cube BoundingVolumeComponent::GetWorldAABB() const
{
    Cube worldBox;
    if (!mBoundingBox) return worldBox;

    Vector3 pos = mOwnerActor->GetPosition();
    float scale = mOwnerActor->GetScale();

    worldBox.min = mBoundingBox->min * scale + pos;
    worldBox.max = mBoundingBox->max * scale + pos;

    return worldBox;
}
