#include "Environment/SkyDomeComponent.h"
#include "Environment/SkyDomeMeshGenerator.h"
#include "Asset/Geometry/VertexArray.h"
#include "Engine/Render/Renderer.h"
#include "Engine/Render/LightingManager.h"
#include "Engine/Core/Actor.h"
#include "Engine/Core/Application.h"
#include "Engine/Render/Renderer.h"
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
