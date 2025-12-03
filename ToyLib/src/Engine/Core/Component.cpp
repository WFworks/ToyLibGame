#include "Engine/Core/Component.h"
#include "Engine/Core/Actor.h"
#include <iostream>

namespace toy {

Component::Component(Actor* a, int order)
: mOwnerActor(a)
, mUpdateOrder(order)
{
    
}

Component::~Component()
{
    
}

void Component::Update(float deltaTime)
{
}

Vector3 Component::GetPosition() const
{
    return mOwnerActor->GetPosition();
}

} // namespace toy
