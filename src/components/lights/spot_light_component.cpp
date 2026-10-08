#include "spot_light_component.hpp"
#include "components/core/transform_component.hpp"
#include "core/logging.hpp"
#include "scene/entity.hpp"
#include "rendering/renderer.hpp"
#include "rendering/render_utils.hpp"

constexpr float NEAR_PLANE = 0.1f;
constexpr float FAR_PLANE = 64.0f;

void SpotLightComponent::LateUpdate(const AppContext &context, float deltaTime)
{
    TransformComponent *transform = this->GetOwner()->GetComponent<TransformComponent>();
    if (!transform)
        return;

    if (!this->castShadows)
        return;

    RenderView view{};
    view.type = ViewType::SHADOW_MAP_SPOT;

    view.nearPlane = NEAR_PLANE;
    view.farPlane = FAR_PLANE;
    view.cameraPosition, transform->GetWorldPosition();

    XMMATRIX viewMatrix = XMMatrixInverse(nullptr, transform->GetRenderMatrix());
    XMStoreFloat4x4(&view.viewMatrix, viewMatrix);

    XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(
        this->outerAngle,
        1,
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

void SpotLightComponent::Render(const RenderView& view, RenderQueue& queue)
{
    if (view.type != ViewType::PRIMARY)
        return;

    TransformComponent *transform = this->GetOwner()->GetComponent<TransformComponent>();
    if (!transform)
        return;

    if (!this->castShadows)
        return;

    SpotLightCommand command{};

    command.innerAngle = this->innerAngle;
    command.outerAngle = this->outerAngle;
    command.colour = this->colour;
    command.direction = transform->GetForward();
    command.intensity = this->intensity;
    command.position = transform->GetRenderPosition();
    command.range = this->range;
    command.castShadows = this->castShadows;
    command.viewProjectionMatrix = this->viewProjectionMatrix;

    queue.Submit(command);
}