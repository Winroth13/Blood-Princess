#include "camera_component.hpp"
#include "components/core/transform_component.hpp"
#include "scene/entity.hpp"
#include "rendering/renderer.hpp"
#include "components/gameplay/shake_component.hpp"

void CameraComponent::Update(const AppContext &context, float deltaTime) {
    TransformComponent *transform = this->GetOwner()->GetComponent<TransformComponent>();
    if (!transform)
        return;

    RenderView view{};
    view.type = ViewType::PRIMARY;

    view.cameraPosition = transform->GetWorldPosition();
    view.nearPlane = this->nearPlane;
    view.farPlane = this->farPlane;


    XMFLOAT3 rotation = transform->GetWorldRotation();
    XMFLOAT3 shakePosition = { 0.0f, 0.0f, 0.0f };

    ShakeComponent *shake = this->GetOwner()->GetComponent<ShakeComponent>();
    if (shake)
    {
        const XMFLOAT3 shakeRotation = shake->GetRotationOffset();
        rotation.x += shakeRotation.x;
        rotation.y += shakeRotation.y;
        rotation.z += shakeRotation.z;

        shakePosition = transform->TransformDirection(shake->GetPositionOffset());
    }

    XMMATRIX viewMatrix = XMMatrixRotationRollPitchYaw(
        XMConvertToRadians(rotation.x),
        XMConvertToRadians(rotation.y),
        XMConvertToRadians(rotation.z)
    );
    viewMatrix *= XMMatrixTranslation(
        view.cameraPosition.x + shakePosition.x,
        view.cameraPosition.y + shakePosition.y,
        view.cameraPosition.z + shakePosition.z
    );
    
    viewMatrix = XMMatrixInverse(nullptr, viewMatrix);
    XMStoreFloat4x4(&view.viewMatrix, viewMatrix);

    XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(
        this->fieldOfView,
        context.renderer->GetAspectRatio(),
        this->nearPlane,
        this->farPlane
    );
    XMStoreFloat4x4(&view.projectionMatrix, projectionMatrix);

    context.renderer->AddView(view);
}