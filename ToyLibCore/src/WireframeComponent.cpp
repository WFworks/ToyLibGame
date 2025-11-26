#include "WireframeComponent.h"
#include "Actor.h"
#include "Application.h"
#include "Renderer.h"
#include "Shader.h"
#include "LightingManager.h"
#include "VertexArray.h"


WireframeComponent::WireframeComponent(Actor* owner, int drawOrder, VisualLayer layer)
: VisualComponent(owner, drawOrder, layer)
, mColor(Vector3(1.f, 1.f, 1.f))
{
    mShader = mOwnerActor->GetApp()->GetRenderer()->GetShader("Solid");
}


void WireframeComponent::Draw()
{
    if (!mIsVisible) return;
    
    auto renderer = mOwnerActor->GetApp()->GetRenderer();
    Matrix4 view = renderer->GetViewMatrix();
    Matrix4 proj = renderer->GetProjectionMatrix();
    
    mShader->SetActive();
    mLightingManager->ApplyToShader(mShader, view);
    mShader->SetMatrixUniform("uViewProj", view * proj);
    mShader->SetVectorUniform("uSolColor", mColor);
    
    
    // WorldマトリックスをShaderに送る
    mShader->SetMatrixUniform("uWorldTransform", mOwnerActor->GetWorldTransform());
    mVertexArray->SetActive();
    glDrawElements(GL_LINE_STRIP,  mVertexArray->GetNumVerts() * 3, GL_UNSIGNED_INT, nullptr);

    /*
    // AABBを表示する
    Matrix4 worldMatrix = mOwnerActor->GetWorldTransform();
    // 列ベクトルからスケール成分を抽出
    float scaleX = Vector3(worldMatrix.mat[0][0], worldMatrix.mat[0][1], worldMatrix.mat[0][2]).Length();
    float scaleY = Vector3(worldMatrix.mat[1][0], worldMatrix.mat[1][1], worldMatrix.mat[1][2]).Length();
    float scaleZ = Vector3(worldMatrix.mat[2][0], worldMatrix.mat[2][1], worldMatrix.mat[2][2]).Length();
    // 平行移動成分
    float transX = worldMatrix.mat[3][0];
    float transY = worldMatrix.mat[3][1];
    float transZ = worldMatrix.mat[3][2];
    
    Matrix4 mat;
    // X軸スケール
    mat.mat[0][0] = scaleX;
    mat.mat[0][1] = 0.0f;
    mat.mat[0][2] = 0.0f;
    mat.mat[0][3] = 0.0f;

    // Y軸スケール
    mat.mat[1][0] = 0.0f;
    mat.mat[1][1] = scaleY;
    mat.mat[1][2] = 0.0f;
    mat.mat[1][3] = 0.0f;

    // Z軸スケール
    mat.mat[2][0] = 0.0f;
    mat.mat[2][1] = 0.0f;
    mat.mat[2][2] = scaleZ;
    mat.mat[2][3] = 0.0f;

    // Translation（平行移動）
    mat.mat[3][0] = transX;
    mat.mat[3][1] = transY;
    mat.mat[3][2] = transZ;
    mat.mat[3][3] = 1.0f;
    
    shader->SetMatrixUniform("uWorldTransform", mat);
    shader->SetVectorUniform("uSolColor", Vector3(1.f, 0.f, 0.f));
    mVertexArray->SetActive();
    glDrawElements(GL_LINE_STRIP, NUM_VERTEX * 3, GL_UNSIGNED_INT, nullptr);
*/
}
