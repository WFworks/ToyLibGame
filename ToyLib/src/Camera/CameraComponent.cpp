#include "Camera/CameraComponent.h"
#include "Engine/Core/Actor.h"
#include "Engine/Render/Renderer.h"
#include "Engine/Core/Application.h"
#include "Physics/ColliderComponent.h"

namespace toy {

CameraComponent::CameraComponent(Actor* a, int updateOrder)
: Component(a, updateOrder)
{
    mCameraActor = std::make_unique<Actor>(GetOwner()->GetApp());
}

void CameraComponent::SetViewMatrix(const Matrix4& view)
{
    GetOwner()->GetApp()->GetRenderer()->SetViewMatrix(view);
}

void CameraComponent::SetCameraPosition(const Vector3& pos)
{
    //GetOwner()->GetApp()->GetRenderer()->SetCameraPosition(pos);
}

void CameraComponent::Update(float deltaTime)
{
    auto inView = GetOwner()->GetApp()->GetRenderer()->GetInvViewMatrix();
    mCameraPosition = inView.GetTranslation();
}

} // namespace toy
