#include "Graphics/VisualComponent.h"
#include "Engine/Core/Actor.h"
#include "Engine/Core/Application.h"
#include "Engine/Render/Renderer.h"
#include "Engine/Render/LightingManager.h"

VisualComponent::VisualComponent(Actor* owner, int drawOrder, VisualLayer layer)
: Component(owner)
, mTexture(nullptr)
, mIsVisible(true)
, mIsBlendAdd(false)
, mLayer(layer)
, mDrawOrder(drawOrder)
, mEnableShadow(false)
{
    mOwnerActor->GetApp()->GetRenderer()->AddVisualComp(this);
    mLightingManager = mOwnerActor->GetApp()->GetRenderer()->GetLightingManager();
    mVertexArray = mOwnerActor->GetApp()->GetRenderer()->GetSpriteVerts();
    
}

VisualComponent::~VisualComponent()
{
    mOwnerActor->GetApp()->GetRenderer()->RemoveVisualComp(this);
}
