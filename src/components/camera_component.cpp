#include "camera_component.hpp"
#include "components/transform_component.hpp"
#include "scene/entity.hpp"
#include "rendering/renderer.hpp"

void CameraComponent::Update(const AppContext &context, float deltaTime) {
    TransformComponent *transform = this->GetOwner()->GetComponent<TransformComponent>();
    if (!transform)
        return;

    RenderView view{};
    view.type = ViewType::PRIMARY;

    view.cameraPosition = transform->GetWorldPosition();
    view.nearPlane = this->nearPlane;
    view.farPlane = this->farPlane;

    XMMATRIX viewMatrix = XMMatrixInverse(nullptr, transform->GetRenderMatrix());
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