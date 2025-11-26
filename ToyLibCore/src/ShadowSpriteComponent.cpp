#include "ShadowSpriteComponent.h"
#include "Actor.h"
#include "Renderer.h"
#include "Shader.h"
#include "Texture.h"
#include "VertexArray.h"
#include "Application.h"
#include "Renderer.h"
#include "LightingManager.h"
#include <memory>

ShadowSpriteComponent::ShadowSpriteComponent(Actor* owner, int drawOrder)
    : VisualComponent(owner, drawOrder)
    , mTexture(nullptr)
    , mScaleWidth(1.0f)
    , mScaleHeight(1.0f)
{
    mLayer = VisualLayer::Effect3D; // 足元に描く
    mShader = mOwnerActor->GetApp()->GetRenderer()->GetShader("Sprite");
    
    mTexture = std::make_shared<Texture>();
    mTexture->CreateAlphaCircle(256, 0.5f, 0.3f, Vector3(0.f, 0.f, 0.f), 0.8f);
}

ShadowSpriteComponent::~ShadowSpriteComponent()
{
   
}

void ShadowSpriteComponent::SetTexture(std::shared_ptr<Texture> tex)
{
    mTexture = tex;
}

void ShadowSpriteComponent::Draw()
{
    if (!mIsVisible || mTexture == nullptr) return;

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    

    float width = static_cast<float>(mTexture->GetWidth()) * mScaleWidth;
    float height = static_cast<float>(mTexture->GetHeight()) * mScaleHeight;
    Matrix4 scale = Matrix4::CreateScale(width * mOffsetScale, height * mOffsetScale*3, 1.0f);

    // 光の方向から回転角を計算（XZ平面）
    Vector3 lightDir = mOwnerActor->GetApp()->GetRenderer()->GetLightingManager()->GetLightDirection();
    lightDir.y = 0.0f;
    if (lightDir.LengthSq() < 0.0001f) lightDir = Vector3(0, 0, 1);
    lightDir.Normalize();
    float angle = atan2f(lightDir.x, lightDir.z);
    Matrix4 rotY = Matrix4::CreateRotationY(angle);

    Matrix4 rotX = Matrix4::CreateRotationX(Math::ToRadians(90.0f));
    Matrix4 trans = Matrix4::CreateTranslation(mOwnerActor->GetPosition() + mOffsetPosition);
    Matrix4 world = scale * rotX * rotY * trans;

    mShader->SetActive();
    auto renderer = mOwnerActor->GetApp()->GetRenderer();
    Matrix4 view = renderer->GetViewMatrix();
    Matrix4 proj = renderer->GetProjectionMatrix();
    mShader->SetMatrixUniform("uViewProj", view * proj);
    mShader->SetMatrixUniform("uWorldTransform", world);
    mTexture->SetActive(0); // ShadowSprite用ユニット
    mShader->SetTextureUniform("uTexture", 0);

    mVertexArray->SetActive();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}
