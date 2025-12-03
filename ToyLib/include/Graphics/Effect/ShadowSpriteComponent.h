#pragma once
#include "Graphics/VisualComponent.h"
#include "Utils/MathUtil.h"

class ShadowSpriteComponent : public VisualComponent
{
public:
    ShadowSpriteComponent(class Actor* owner, int drawOrder = 10);
    ~ShadowSpriteComponent();

    void Draw() override;

    void SetTexture(std::shared_ptr<class Texture> tex) override;
    void SetScale(float width, float height) { mScaleWidth = width; mScaleHeight = height; }
    
    void SetOffsetPosition(const Vector3& vPos) { mOffsetPosition = vPos; }
    void SetOffsetScale(const float f) { mOffsetScale = f; }

private:
    std::shared_ptr<class Texture> mTexture;
    float mScaleWidth;
    float mScaleHeight;
    Vector3 mOffsetPosition;
    float mOffsetScale;
};
