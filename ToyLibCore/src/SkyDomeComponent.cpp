#include "SkyDomeComponent.h"
#include "SkyDomeMeshGenerator.h"
#include "VertexArray.h"
#include "Renderer.h"
#include "LightingManager.h"
#include "Actor.h"
#include "Application.h"
#include "Renderer.h"
#include <algorithm>

SkyDomeComponent::SkyDomeComponent(Actor* a)
: Component(a)
{
    mSkyVAO = SkyDomeMeshGenerator::CreateSkyDomeVAO(32, 16, 1.0f);
    mOwnerActor->GetApp()->GetRenderer()->RegisterSkyDome(this);
    mShader = mOwnerActor->GetApp()->GetRenderer()->GetShader("SkyDome");
}


void SkyDomeComponent::Draw()
{

    if (!mSkyVAO || !mShader) return;
    

}


void SkyDomeComponent::Update(float deltaTime)
{

}
