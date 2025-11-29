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

}
