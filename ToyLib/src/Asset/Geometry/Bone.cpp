#include "Asset/Geometry/Bone.h"
#include <iostream>
#include <cassert>

namespace toy {

// Bone処理用
void VertexBoneData::AddBoneData(unsigned int BoneID, float Weight)
{
    // 頂点あたりBone４本まで設定
    for (unsigned int i = 0 ; i < NUM_BONES_PER_VERTEX ; i++)
    {
        if (Weights[i] == 0.0)
        {
            IDs[i]     = BoneID;
            Weights[i] = Weight;
            return;
        }
    }
    assert(0);
}

} // namespace toy
