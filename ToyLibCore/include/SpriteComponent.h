#pragma once

#include "VisualComponent.h"

class SpriteComponent : public VisualComponent
{
public:
    SpriteComponent(class Actor* a, int drawOrder, VisualLayer layer = VisualLayer::UI);
    ~SpriteComponent();

    void Draw() override;

    void SetScale(float w, float h) { mScaleWidth = w; mScaleHeight = h; }
    void SetTexture(std::shared_ptr<class Texture> tex) override;

private:
    float mScaleWidth;
    float mScaleHeight;
    int mTexWidth;
    float mTexHeight;
    int mScreenWidth;
    int mScreenHeight;

};



