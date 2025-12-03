#pragma once

#include <string>
#include <assimp/scene.h>

// アニメーションクリップ情報
struct AnimationClip
{
    std::string mName;              // アニメーション名（Node名 or ファイル内での識別用）
    const aiAnimation* mAnimation;  // Assimpから渡された生データ
    float mDuration;                // 再生時間（Ticks）
    float mTicksPerSecond;          // 1秒あたりのTick数
};
