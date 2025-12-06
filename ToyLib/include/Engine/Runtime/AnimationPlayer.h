#pragma once

#include "Utils/MathUtil.h"
#include <assimp/scene.h>
#include <vector>
#include <memory>

namespace toy {

//-------------------------------------------------------------
// BlendInfo
// ・アニメーション間のブレンド（遷移）に使う情報
// ・fromAnim → 現在のアニメーション
// ・toAnim   → 遷移先のアニメーション
// ・blendDuration の間、0→1に向かって補間
//-------------------------------------------------------------
struct BlendInfo
{
    const aiAnimation* fromAnim = nullptr; // ブレンド元アニメ
    const aiAnimation* toAnim   = nullptr; // ブレンド先アニメ
    float blendDuration = 0.3f;            // ブレンドに要する時間
    float blendTime     = 0.0f;            // ブレンド経過時間
    bool  isBlending    = false;           // ブレンド中かどうか
};


//-------------------------------------------------------------
// AnimationPlayer
// ・Mesh に紐づいたスケルトンアニメーションの再生制御クラス
// ・Assimp の aiAnimation を使ってポーズ計算し、
//   各ボーンの最終行列（mFinalMatrices）を生成する。
// ・ループ再生／一回再生／アニメーションブレンドをサポート。
//-------------------------------------------------------------
class AnimationPlayer
{
public:
    // Mesh（ボーン構造を持つ）を共有して使う
    AnimationPlayer(std::shared_ptr<class Mesh> mesh);
    
    //---------------------------------------------------------
    // 更新
    //---------------------------------------------------------
    
    // 時間を進めて現在のアニメーション（およびブレンド）を更新
    void Update(float deltaTime);
    
    
    //---------------------------------------------------------
    // 再生制御
    //---------------------------------------------------------
    
    // 指定 ID のアニメーションを再生
    //   animID : Mesh 内部でのアニメーションID
    //   loop   : ループ再生するかどうか
    void Play(int animID, bool loop = true);
    
    // 再生速度（1.0 が等倍、2.0 で2倍速など）
    void SetPlayRate(float rate) { mPlayRate = rate; }
    
    // 一時停止／再開
    void Pause(bool paused) { mIsPaused = paused; }
    
    
    //---------------------------------------------------------
    // 行列取得
    //---------------------------------------------------------
    
    // 各ボーンの最終行列（スキニングに使用）
    const std::vector<Matrix4>& GetFinalMatrices() const { return mFinalMatrices; }
    
    
    //---------------------------------------------------------
    // 一回再生・ブレンド再生
    //---------------------------------------------------------
    
    // animID を一度だけ再生し、終了後に nextAnimID へ遷移
    void PlayOnce(int animID, int nextAnimID);
    
    // fromAnimID から toAnimID へ duration 秒かけてブレンド
    void PlayBlend(int fromAnimID, int toAnimID, float duration);
    
    
    //---------------------------------------------------------
    // 状態問い合わせ
    //---------------------------------------------------------
    
    // 一回再生のアニメが終了したか
    bool IsFinished() const { return mIsFinished; }
    
    // 現在のアニメーションがループ再生か
    bool IsLooping() const { return mIsLooping; }
    
    
private:
    //---------------------------------------------------------
    // 再生対象
    //---------------------------------------------------------
    
    std::shared_ptr<class Mesh> mMesh; // ボーン・アニメを持つ Mesh
    
    int   mAnimID;       // 再生中のアニメーションID
    float mPlayTime;     // 現在の再生時間
    float mPlayRate;     // 再生速度（1.0 = 等倍）
    bool  mIsLooping;    // ループ再生中か
    bool  mIsPaused;     // 一時停止中か
    
    int   mNextAnimID;   // PlayOnce 用：再生完了後に切り替えるID
    bool  mIsFinished;   // 一回再生が終了したかどうか
    
    // スキニング用 最終ボーン行列
    std::vector<Matrix4> mFinalMatrices;
    
    // ブレンド情報（遷移中の補間状態など）
    BlendInfo mBlend;
    
    
    //---------------------------------------------------------
    // 内部ヘルパー
    //---------------------------------------------------------
    
    // 2つの行列を線形補間（簡易ブレンド用）
    Matrix4 LerpMatrix(const Matrix4& a, const Matrix4& b, float t);
    
    // aiAnimation* から、対応する AnimationClip / 内部テーブルのインデックスを探す
    int FindClipIndex(const aiAnimation* anim) const;
};

} // namespace toy
