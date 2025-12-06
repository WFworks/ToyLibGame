#include "Graphics/Sprite/SpriteComponent.h"
#include "Asset/Material/Texture.h"
#include "Engine/Render/Shader.h"
#include "Engine/Render/LightingManager.h"
#include "Asset/Geometry/VertexArray.h"
#include "Engine/Core/Application.h"
#include "Engine/Render/Renderer.h"
#include "Engine/Core/Actor.h"
#include <GL/glew.h>

namespace toy {

SpriteComponent::SpriteComponent(Actor* a, int drawOrder, VisualLayer layer)
: VisualComponent(a, drawOrder, layer)
, mScaleWidth(1.0f)
, mScaleHeight(1.0f)
, mTexWidth(0)
, mTexHeight(0)
{
    mDrawOrder = drawOrder;
    mShader = GetOwner()->GetApp()->GetRenderer()->GetShader("Sprite");
    mScreenWidth = GetOwner()->GetApp()->GetRenderer()->GetScreenWidth();
    mScreenHeight = GetOwner()->GetApp()->GetRenderer()->GetScreenHeight();
}

SpriteComponent::~SpriteComponent()
{
}

void SpriteComponent::SetTexture(std::shared_ptr<Texture> tex)
{
    VisualComponent::SetTexture(tex);
    if (tex)
    {
        mTexWidth  = tex->GetWidth();
        mTexHeight = tex->GetHeight();
    }
    else
    {
        mTexWidth  = 0;
        mTexHeight = 0;
    }
}

void SpriteComponent::Draw()
{
    if (!mIsVisible || mTexture == nullptr) return;

    // ---- ブレンド/深度設定 ----
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(mIsBlendAdd ? GL_ONE : GL_SRC_ALPHA,
        mIsBlendAdd ? GL_ONE : GL_ONE_MINUS_SRC_ALPHA);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    auto* app = GetOwner()->GetApp();
    auto* renderer = app->GetRenderer();

    // ウィンドウの DPI スケール（100%→1.0, 150%→1.5 など）
    float dpi = renderer->GetWindowDisplayScale();

    // テクスチャの「最終的な画面上サイズ」（ピクセルベース）
    float texW = static_cast<float>(mTexWidth);
    float texH = static_cast<float>(mTexHeight);
    float width = texW * mScaleWidth * dpi;
    float height = texH * mScaleHeight * dpi;

    // 論理座標の位置 → DPI を掛けてピクセル座標に変換
    Vector3 pos = GetOwner()->GetPosition();   // ここは論理 1280x720 ベース想定
    pos.x *= dpi;
    pos.y *= dpi;
    // pos.z はたぶん 0 のままで OK（UI なら）

    // 2D 用 World 行列（スケール＋平行移動）
    Matrix4 world = Matrix4::CreateScale(width, height, 1.0f);
    world *= Matrix4::CreateTranslation(pos);

    // ---- シェーダ設定 ----
    mShader->SetActive();

    float screenW = renderer->GetScreenWidth();
    float screenH = renderer->GetScreenHeight();

    Matrix4 viewProj = Matrix4::CreateSimpleViewProj(screenW, screenH);
    mShader->SetMatrixUniform("uViewProj", viewProj);

    mTexture->SetActive(0);
    mShader->SetTextureUniform("uTexture", 0);
    mShader->SetMatrixUniform("uWorldTransform", world);

    Matrix4 view = renderer->GetViewMatrix();
    mLightingManager->ApplyToShader(mShader, view);

    // ---- 描画 ----
    mVertexArray->SetActive();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    // ---- 戻す ----
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
}

} // namespace toy
