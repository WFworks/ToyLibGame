#pragma once

#include "MathUtils.h"
#include <assimp/scene.h>
#include <vector>
#include <memory>

struct BlendInfo
{
    const aiAnimation* fromAnim = nullptr;
    const aiAnimation* toAnim = nullptr;
    float blendDuration = 0.3f;
    float blendTime = 0.0f;
    bool isBlending = false;
};

class AnimationPlayer
{
public:
    AnimationPlayer(std::shared_ptr<class Mesh> mesh);

    void Update(float deltaTime);
    void Play(int animID, bool loop = true);
    void SetPlayRate(float rate) { mPlayRate = rate; }
    void Pause(bool paused) { mIsPaused = paused; }

    const std::vector<Matrix4>& GetFinalMatrices() const { return mFinalMatrices; };
    
    void PlayOnce(int animID, int nextAnimID);
    void PlayBlend(int fromAnimID, int toAnimID, float duration);
    
    bool IsFinished() const { return mIsFinished; }
    bool IsLooping() const { return mIsLooping; }

private:
    std::shared_ptr<class Mesh> mMesh;
    int mAnimID;
    float mPlayTime;
    float mPlayRate;
    bool mIsLooping;
    bool mIsPaused;
    
    int mNextAnimID;
    bool mIsFinished;

    std::vector<Matrix4> mFinalMatrices;
    BlendInfo mBlend;
    
    Matrix4 LerpMatrix(const Matrix4& a, const Matrix4& b, float t);
    int FindClipIndex(const aiAnimation* anim) const;
};

