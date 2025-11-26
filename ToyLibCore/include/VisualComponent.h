#pragma once

#include "Component.h"
#include "Renderer.h"


class VisualComponent : public Component
{
public:
    VisualComponent(class Actor* owner, int drawOrder, VisualLayer layer = VisualLayer::Effect3D);
    virtual ~VisualComponent();

    virtual void Draw() = 0;
    virtual void DrawShadow() {}

    virtual void SetTexture(std::shared_ptr<class Texture> tex) { mTexture = tex; }
    std::shared_ptr<class Texture> GetTexture() const { return mTexture; }

    void SetVisible(bool v) { mIsVisible = v; }
    bool IsVisible() const { return mIsVisible; }

    void SetBlendAdd(bool b) { mIsBlendAdd = b; }
    bool IsBlendAdd() const { return mIsBlendAdd; }

    void SetLayer(VisualLayer layer) { mLayer = layer; }
    VisualLayer GetLayer() const { return mLayer; }
    

    int GetDrawOrder() const { return mDrawOrder; }
    void SetDrawOrder(int order) { mDrawOrder = order; }
    
    void SetShader(std::shared_ptr<class Shader> shader) { mShader = shader; }
    void SetLightingManager(std::shared_ptr<LightingManager> light) { mLightingManager = light; }

    bool IsEnableShadow() const { return mEnableShadow; }
protected:
    std::shared_ptr<class Texture> mTexture;
    std::shared_ptr<class Shader> mShader;
    std::shared_ptr<class LightingManager> mLightingManager;
    bool mIsVisible;
    bool mIsBlendAdd;
    VisualLayer mLayer;
    int mDrawOrder;
    bool mEnableShadow;
    std::shared_ptr<class VertexArray> mVertexArray;
    
};
