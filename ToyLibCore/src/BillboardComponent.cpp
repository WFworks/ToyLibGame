#include "BillboardComponent.h"
#include "Texture.h"
#include "Shader.h"
#include "LightingManager.h"
#include "VertexArray.h"
#include "Application.h"
#include "Actor.h"
#include "Renderer.h"

#include <GL/glew.h>

BillboardComponent::BillboardComponent(class Actor* a, int drawOrder)
: VisualComponent(a, drawOrder, VisualLayer::Effect3D)
, mScale(1.0f)
{
    mShader = mOwnerActor->GetApp()->GetRenderer()->GetShader("Mesh");
}

BillboardComponent::~BillboardComponent()
{
}

void BillboardComponent::Draw()
{
    if (!mIsVisible || !mTexture) return;

    if (mIsBlendAdd)
    {
        glBlendFunc(GL_ONE, GL_ONE);
    }
    
    auto renderer = mOwnerActor->GetApp()->GetRenderer();
    Matrix4 view = renderer->GetViewMatrix();
    Matrix4 proj = renderer->GetProjectionMatrix();
    
    
    // カメラと位置取得
    Vector3 pos = mOwnerActor->GetPosition();
    Matrix4 invView = mOwnerActor->GetApp()->GetRenderer()->GetInvViewMatrix();
    Vector3 cameraPos = invView.GetTranslation();

    // 回転角（Y軸）
    Vector3 toCamera = pos - cameraPos;
    toCamera.y = 0.0f;
    toCamera.Normalize();

    float angle = atan2f(toCamera.x, toCamera.z);
    Matrix4 rotY = Matrix4::CreateRotationY(angle);

    // スケール＋平行移動
    float scale = mScale * mOwnerActor->GetScale();
    Matrix4 scaleMat = Matrix4::CreateScale(mTexture->GetWidth() * scale,
                                            mTexture->GetHeight() * scale, 1.0f);
    Matrix4 translate = Matrix4::CreateTranslation(pos);

    mShader->SetActive();
    mLightingManager->ApplyToShader(mShader, view);

    // 最終行列
    Matrix4 world = scaleMat * rotY * translate;
    mShader->SetMatrixUniform("uWorldTransform", world);
    mShader->SetMatrixUniform("uViewProj", view * proj);
    mTexture->SetActive(0);
    mShader->SetTextureUniform("uTexture", 0);
    
    // VAO有効化
    mVertexArray->SetActive();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    if (mIsBlendAdd)
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
}
