#include "Graphics/VisualComponent.h"
#include "Engine/Core/Actor.h"
#include "Engine/Core/Application.h"
#include "Engine/Render/Renderer.h"
#include "Engine/Render/LightingManager.h"

namespace toy {

VisualComponent::VisualComponent(Actor* owner, int drawOrder, VisualLayer layer)
: Component(owner)
, mTexture(nullptr)
, mIsVisible(true)
, mIsBlendAdd(false)
, mLayer(layer)
, mDrawOrder(drawOrder)
, mEnableShadow(false)
{
    GetOwner()->GetApp()->GetRenderer()->AddVisualComp(this);
    mLightingManager = GetOwner()->GetApp()->GetRenderer()->GetLightingManager();
    mVertexArray = GetOwner()->GetApp()->GetRenderer()->GetSpriteVerts();
    
}

VisualComponent::~VisualComponent()
{
    GetOwner()->GetApp()->GetRenderer()->RemoveVisualComp(this);
}

} // namespace toy
