#pragma once
#include "VisualComponent.h"
#include "MathUtils.h"

class ShadowSpriteComponent : public VisualComponent
{
public:
    ShadowSpriteComponent(class Actor* owner, int drawOrder = 10);
    ~ShadowSpriteComponent();

    void Draw() override;

    void SetTexture(std::shared_ptr<class Texture> tex) override;
    void SetScale(float width, float height) { mScaleWidth = width; mScaleHeight = height; }
    
    void SetOffsetPosition(const Vector3 v) { mOffsetPosition = v; }
    void SetOffsetScale(const float f) { mOffsetScale = f; }

private:
    std::shared_ptr<class Texture> mTexture;
    float mScaleWidth;
    float mScaleHeight;
    Vector3 mOffsetPosition;
    float mOffsetScale;
};
