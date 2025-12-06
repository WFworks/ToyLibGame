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

    auto* renderer = GetOwner()->GetApp()->GetRenderer();

    // 物理解像度
    float sw = renderer->GetScreenWidth();
    float sh = renderer->GetScreenHeight();

    // 論理解像度
    float vw = renderer->GetVirtualWidth();
    float vh = renderer->GetVirtualHeight();

    // 0除算回避
    if (vw <= 0.0f) vw = sw;
    if (vh <= 0.0f) vh = sh;

    // 論理→物理変換は「小さい方」に合わせる（アスペクト比維持）
    float sx = sw / vw;
    float sy = sh / vh;
    float scale = (sx < sy) ? sx : sy;

    // サイズ
    float texW = static_cast<float>(mTexWidth);
    float texH = static_cast<float>(mTexHeight);
    float width  = texW * mScaleWidth  * scale;
    float height = texH * mScaleHeight * scale;

    // 位置
    Vector3 pos = GetOwner()->GetPosition();
    pos.x *= scale;
    pos.y *= scale;

    // World / ViewProj
    Matrix4 world = Matrix4::CreateScale(width, height, 1.0f);
    world *= Matrix4::CreateTranslation(pos);

    Matrix4 viewProj = Matrix4::CreateSimpleViewProj(sw, sh);

    mShader->SetActive();
    mShader->SetMatrixUniform("uViewProj", viewProj);
    mShader->SetMatrixUniform("uWorldTransform", world);

    mTexture->SetActive(0);
    mShader->SetTextureUniform("uTexture", 0);

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
