#include "VisualComponent.h"
#include "Actor.h"
#include "Application.h"
#include "Renderer.h"
#include "LightingManager.h"

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
