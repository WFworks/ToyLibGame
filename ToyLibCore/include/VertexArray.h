#pragma once

#include "MathUtils.h"
#include <memory>
#include <vector>

// 頂点データ管理
class VertexArray
{
public:

    // コンストラクタ（フォーマットごとに３種類）
    
    // 主にスプライト用(4)
    VertexArray(const float* verts,
                unsigned int num_verts,
                const unsigned int* indices,
                unsigned int num_indices);

    // メッシュ用(6)
    VertexArray(unsigned int num_verts,
                const float* verts,
                const float* norms,
                const float* uvs,
                unsigned int num_indices,
                const unsigned int* indices);


    // アニメーションメッシュ用(8)
    VertexArray(unsigned int num_verts,
                const float* verts,
                const float* norms,
                const float* uvs,
                const unsigned int* boneids,
                const float* weights,
                unsigned int num_indices,
                const unsigned int* indices);
    
    // 雨やフルスクリーンエフェクト用(5)
    VertexArray(const float* verts,
                unsigned int numVerts,
                const unsigned int* indices,
                unsigned int numIndices,
                bool isVec2Only);
    
    // デストラクタ
    virtual ~VertexArray();
    
    // 描画用にBind
    void SetActive();
    // テクスチャIDを設定
    void SetTextureID(unsigned int id) { mTextureID = id; }
    unsigned int GetTextureID() const { return mTextureID; }

    // インデックス数
    unsigned int GetNumIndices() const { return mNumIndices; }
    // 頂点数
    unsigned int GetNumVerts() const { return mNumVerts; }
    // 面データ
    const std::vector<struct Polygon>& GetPolygons() const { return mPolygons; }
    std::vector<struct Polygon> GetWorldPolygons(const Matrix4& worldTransform) const;

private:
    // 頂点の
    unsigned int mNumVerts;
    // インデックスの数
    unsigned int mNumIndices;
    // 頂点バッファVBO
    unsigned int mVertexBuffer[5];

    
    // 頂点データID
    unsigned int mVertexBufferID;
    // インデックスバッファID
    unsigned int mIndexBufferID;
    // テクスチャID
    unsigned int mTextureID;
    
    // ポリゴンデータ格納（計算用途）
    //std::vector<std::unique_ptr<struct Polygon>> mPolygons;
    std::vector<Polygon> mPolygons;
    
    void CreatePolygons(const float* verts, const unsigned int* indices, const unsigned int num);
    
};


