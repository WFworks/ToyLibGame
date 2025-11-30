#include "Camera/CameraComponent.h"
#include "Engine/Core/Actor.h"
#include "Engine/Render/Renderer.h"
#include "Engine/Core/Application.h"
#include "Physics/ColliderComponent.h"

CameraComponent::CameraComponent(Actor* a, int updateOrder)
: Component(a, updateOrder)
{
    mCameraActor = std::make_unique<Actor>(mOwnerActor->GetApp());
}

void CameraComponent::SetViewMatrix(const Matrix4& view)
{
    mOwnerActor->GetApp()->GetRenderer()->SetViewMatrix(view);
}

void CameraComponent::SetCameraPosition(const Vector3& pos)
{
    //mOwnerActor->GetApp()->GetRenderer()->SetCameraPosition(pos);
}

void CameraComponent::Update(float deltaTime)
{
    auto inView = mOwnerActor->GetApp()->GetRenderer()->GetInvViewMatrix();
    mCameraPosition = inView.GetTranslation();
}


