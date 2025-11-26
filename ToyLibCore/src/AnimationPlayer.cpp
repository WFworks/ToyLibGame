#include "Mesh.h"
#include "AnimationPlayer.h"
#include <assimp/scene.h>
#include <iostream>

AnimationPlayer::AnimationPlayer(std::shared_ptr<Mesh> mesh)
: mMesh(mesh)
, mAnimID(0)
, mPlayTime(0.0f)
, mPlayRate(1.0f)
, mIsLooping(true)
, mIsPaused(false)
, mNextAnimID(-1)
, mIsFinished(false)
{
}

void AnimationPlayer::Play(int animID, bool loop)
{
    // 同じ内容なら再生しなおさない
    if (mAnimID == animID && mIsLooping == loop && !mIsPaused) return;

    mAnimID = animID;
    mPlayTime = 0.0f;
    mIsLooping = loop;
    mIsPaused = false;
    mIsFinished = false;
    mNextAnimID = -1;
}

void AnimationPlayer::Update(float deltaTime)
{
    if (!mMesh || mIsPaused) return;

    // --- ① ブレンド中の処理 ---
    if (mBlend.isBlending)
    {
        float t = mBlend.blendTime / mBlend.blendDuration;
        t = std::clamp(t, 0.0f, 1.0f);

        std::vector<Matrix4> poseA, poseB;
        float timeA = mPlayTime * mBlend.fromAnim->mTicksPerSecond;
        float timeB = mPlayTime * mBlend.toAnim->mTicksPerSecond;

        mMesh->ComputePoseAtTime(fmod(timeA, mBlend.fromAnim->mDuration), mBlend.fromAnim, poseA);
        mMesh->ComputePoseAtTime(fmod(timeB, mBlend.toAnim->mDuration), mBlend.toAnim, poseB);

        mFinalMatrices.resize(poseA.size());
        for (size_t i = 0; i < poseA.size(); ++i)
        {
            mFinalMatrices[i] = LerpMatrix(poseA[i], poseB[i], t);
        }

        mBlend.blendTime += deltaTime;
        mPlayTime += deltaTime;

        if (mBlend.blendTime >= mBlend.blendDuration)
        {
            mBlend.isBlending = false;
            Play(FindClipIndex(mBlend.toAnim));  // ←次の通常再生へ
        }

        return; // ブレンド処理で終了！
    }

    // --- ② 通常のアニメ再生 ---
    const auto& clips = mMesh->GetAnimationClips();
    if (mAnimID < 0 || mAnimID >= static_cast<int>(clips.size())) return;

    const aiAnimation* anim = clips[mAnimID].mAnimation;
    float ticksPerSecond = clips[mAnimID].mTicksPerSecond;

    float timeInTicks = mPlayTime * mPlayRate * ticksPerSecond;
    float animTime = fmod(timeInTicks, anim->mDuration);

    if (!mIsLooping && timeInTicks >= anim->mDuration)
    {
        mIsFinished = true;
        if (mNextAnimID >= 0)
        {
            Play(mNextAnimID);
        }
        return;
    }

    mMesh->ComputePoseAtTime(animTime, anim, mFinalMatrices);
    mPlayTime += deltaTime;
}

void AnimationPlayer::PlayOnce(int animID, int nextAnimID)
{
    mAnimID = animID;
    mPlayTime = 0.0f;
    mIsLooping = false;
    mIsPaused = false;
    mIsFinished = false;
    mNextAnimID = nextAnimID;
}

void AnimationPlayer::PlayBlend(int fromAnimID, int toAnimID, float duration)
{
    const auto& clips = mMesh->GetAnimationClips();
    if (fromAnimID < 0 || fromAnimID >= clips.size()) return;
    if (toAnimID < 0 || toAnimID >= clips.size()) return;

    mBlend.fromAnim = clips[fromAnimID].mAnimation;
    mBlend.toAnim = clips[toAnimID].mAnimation;
    mBlend.blendDuration = duration;
    mBlend.blendTime = 0.0f;
    mBlend.isBlending = true;
}

Matrix4 AnimationPlayer::LerpMatrix(const Matrix4& a, const Matrix4& b, float t)
{
    Matrix4 result;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            result.mat[i][j] = a.mat[i][j] * (1.0f - t) + b.mat[i][j] * t;
    return result;
}

int AnimationPlayer::FindClipIndex(const aiAnimation* anim) const
{
    const auto& clips = mMesh->GetAnimationClips();
    for (size_t i = 0; i < clips.size(); ++i)
    {
        if (clips[i].mAnimation == anim)
        {
            return static_cast<int>(i);
        }
    }
    return -1; // 見つからない場合
}
