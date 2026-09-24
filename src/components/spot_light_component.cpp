#include "spot_light_component.hpp"
#include "components/transform_component.hpp"
#include "core/logging.hpp"
#include "scene/entity.hpp"
#include "rendering/renderer.hpp"
#include "rendering/render_utils.hpp"

constexpr float NEAR_PLANE = 0.1f;
constexpr float FAR_PLANE = 512.0f;

void SpotLightComponent::Update(const AppContext &context, float deltaTime)
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
        context.renderer->GetAspectRatio(),
        NEAR_PLANE,
        FAR_PLANE
    );
    XMStoreFloat4x4(&view.projectionMatrix, projectionMatrix);

    context.renderer->AddView(view);
}

void SpotLightComponent::Render(const RenderView& view, RenderQueue& queue)
{
    TransformComponent *transform = this->GetOwner()->GetComponent<TransformComponent>();
    if (!transform)
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

    queue.Submit(command);
}