#pragma once

#include "Utils/MathUtil.h"
#include <cstring>

// ４本まで
const unsigned int NUM_BONES_PER_VERTEX = 4;

namespace toy {

// 頂点ごとのBoneデータ
struct VertexBoneData
{
    // コンストラクタ
    VertexBoneData()
    {
        Reset();
    };
    
    // 頂点に影響あるBoneのID
    unsigned int IDs[NUM_BONES_PER_VERTEX];
    // 影響するウェイト
    float Weights[NUM_BONES_PER_VERTEX];
    
    
    // ゼロにリセット
    void Reset()
    {
        for(int i = 0; i < NUM_BONES_PER_VERTEX; i++)
        {
            IDs[i] = 0;
            Weights[i] = 0.0f;
        }
        
    }
    
    // 頂点データにボーンとウェイトを追加
    void AddBoneData(unsigned int BoneID, float Weight);
};


// Bone情報格納用
struct BoneInfo
{
    Matrix4 BoneOffset;
    Matrix4 FinalTransformation;
    
    
    BoneInfo()
    {
        memset(&BoneOffset, 0, sizeof(BoneOffset));
        memset(&FinalTransformation, 0, sizeof(FinalTransformation));
        
    }
};

} // namespace toy
