#pragma once
#include "AnimationPlayer.h"
#include "MathUtils.h"
#include "MeshComponent.h"
#include <vector>
#include <memory>

const size_t MAX_SKELETON_BONES = 96;


// スケルタルアニメーションメッシュを管理
class SkeletalMeshComponent : public MeshComponent
{
public:
    // コンストラクタ
    SkeletalMeshComponent(class Actor* a, int drawOrder = 100, VisualLayer layer = VisualLayer::Effect3D);
    
    // 描画のOverride
    void Draw() override;
    void DrawShadow() override;
    // UpdateのOverride
    void Update(float deltaTime) override;
    
    // 再生するモーションのID
    void SetAnimID(const unsigned int animID, const bool mode) override;
    
    void SetMesh(std::shared_ptr<class Mesh> m) override;
    
    class AnimationPlayer* GetAnimPlayer() { return mAnimPlayer.get(); }
    

private:
    // アニメーション時間
    float mAnimTime;
    std::unique_ptr<class AnimationPlayer> mAnimPlayer;

};

