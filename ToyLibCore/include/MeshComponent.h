#pragma once

#include "VisualComponent.h"
#include "MathUtils.h"
#include <memory>
//#include <cstddef>

// Meshを管理するComponent
class MeshComponent : public VisualComponent
{
public:
    MeshComponent(class Actor* a, int drawOrder = 100, VisualLayer layer = VisualLayer::Effect3D, bool isSkeletal = false);
    virtual ~MeshComponent();
        
    // 描画 override
    virtual void Draw();
    virtual void DrawShadow();
    
    virtual void SetMesh(std::shared_ptr<class Mesh> m) { mMesh = m; }              // メッシュセット
    void SetTextureIndex(unsigned int index) { mTextureIndex = index; }    // テクスチャGetter
   
    bool GetIsSkeletal() const { return mIsSkeletal; }
    std::shared_ptr<VertexArray> GetVertexArray(int id) const;
    
    void SetToonRender(const bool t, const float f = 1.05f) { mIsToon = t; mContourFactor = f; }
    void SetContourFactor(const float f) { mContourFactor = f; }
    
    bool GetToon() const { return mIsToon; }
    
    // 再生するモーションのID（SkerltalMeshでオーバーライドする。インターフェース確保のため）
    virtual void SetAnimID(const unsigned int animID, const bool mode) {}
    
protected:
    std::shared_ptr<class Mesh>  mMesh;      // メッシュ
    unsigned int mTextureIndex;    // TextureID
    

    //bool mIsVisible;
    bool mIsSkeletal;

    std::shared_ptr<class Texture> mShadowMapTexture;
    
    std::shared_ptr<class LightingManager> mLightingManger;
    std::shared_ptr<class Shader> mShader;
    std::shared_ptr<class Shader> mShadowShader;
    
    
    // 輪郭強調
    bool mIsToon;
    float mContourFactor;
};

