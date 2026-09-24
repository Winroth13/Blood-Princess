#include "directional_light_component.hpp"
#include "components/transform_component.hpp"
#include "core/logging.hpp"
#include "scene/entity.hpp"
#include "rendering/renderer.hpp"
#include "rendering/render_utils.hpp"

constexpr float NEAR_PLANE = 1.0f;
constexpr float FAR_PLANE = 1024.0f;
constexpr float VIEW_WIDTH = 128.0f;
constexpr float VIEW_HEIGHT = 128.0f;
constexpr float DISTANCE_AWAY = 512.0f;

void DirectionalLightComponent::LateUpdate(const AppContext &context, float deltaTime) {
    TransformComponent *transform = this->GetOwner()->GetComponent<TransformComponent>();
    if (!transform)
        return;

    RenderView view{};
    view.type = ViewType::SHADOW_MAP_DIRECTIONAL;

    view.nearPlane = NEAR_PLANE;
    view.farPlane = FAR_PLANE;
    
    XMVECTOR primaryCameraPosition = XMLoadFloat3(
        &RenderUtils::GetView(
            context.renderer->GetViews(),
            ViewType::PRIMARY
        )->cameraPosition
    );

    XMFLOAT3 forwardVector = transform->GetForward();
    XMVECTOR positionOffset = XMVectorScale(
        XMLoadFloat3(&forwardVector),
        -DISTANCE_AWAY
    );

    XMVECTOR lightCameraPosition = XMVectorAdd(primaryCameraPosition, positionOffset);

    XMStoreFloat3(&view.cameraPosition, lightCameraPosition);
     
    XMFLOAT4 quaternionRotation = transform->GetWorldRotationQuaternion();
    XMMATRIX matrix = XMMatrixMultiply(
        XMMatrixRotationQuaternion(
            XMLoadFloat4(&quaternionRotation)
        ),
        XMMatrixTranslationFromVector(lightCameraPosition)
    );

    XMMATRIX viewMatrix = XMMatrixInverse(nullptr, matrix);
    XMStoreFloat4x4(&view.viewMatrix, viewMatrix);

    XMMATRIX projectionMatrix = XMMatrixOrthographicLH(
        VIEW_WIDTH,
        VIEW_HEIGHT,
        NEAR_PLANE,
        FAR_PLANE
    );
    XMStoreFloat4x4(&view.projectionMatrix, projectionMatrix);

    XMStoreFloat4x4(
        &this->viewProjectionMatrix,
        XMMatrixMultiply(
            viewMatrix,
            projectionMatrix
        )
    );

    context.renderer->AddView(view);
}

void DirectionalLightComponent::Render(const RenderView& view, RenderQueue& queue)
{
    TransformComponent *transform = this->GetOwner()->GetComponent<TransformComponent>();
    if (!transform)
        return;

	DirectionalLightCommand command{};

	command.colour = this->colour;
    command.castShadows = this->castShadows;
	command.direction = transform->GetForward();
	command.intensity = this->intensity;
    command.viewProjectionMatrix = this->viewProjectionMatrix;

	queue.Submit(command);
}