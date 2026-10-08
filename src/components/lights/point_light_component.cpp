#include "point_light_component.hpp"
#include "components/core/transform_component.hpp"
#include "core/logging.hpp"
#include "scene/entity.hpp"
#include "rendering/renderer.hpp"

constexpr float NEAR_PLANE = 0.1f;
constexpr float FAR_PLANE = 64.0f;

constexpr DirectX::XMVECTOR CAMERA_CUBEMAP_ANGLES[] =
{
    {0, DirectX::XM_PIDIV2, 0},					// Pos X
    {0, -DirectX::XM_PIDIV2, 0},				// Neg X
    {-DirectX::XM_PIDIV2, 0, 0},				// Pos Y
    {DirectX::XM_PIDIV2, 0, 0},					// Neg Y
    {0, 0, 0},									// Pos Z
    {0, DirectX::XM_PI, 0},						// Neg Z
};

void PointLightComponent::LateUpdate(const AppContext &context, float deltaTime) 
{
    TransformComponent *transform = this->GetOwner()->GetComponent<TransformComponent>();
    if (!transform)
        return;

    currentShadowIndex = 0;

    if (!this->castShadows)
        return;

    RenderView view{};
    view.type = ViewType::SHADOW_MAP_POINT;

    view.nearPlane = NEAR_PLANE;
    view.farPlane = FAR_PLANE;
    view.cameraPosition = transform->GetRenderPosition();

    XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(
        DirectX::XM_PIDIV2,
        1,
        NEAR_PLANE,
        FAR_PLANE
    );
    XMStoreFloat4x4(&view.projectionMatrix, projectionMatrix);

    XMMATRIX translationMatrix =
        XMMatrixTranslationFromVector(
            XMLoadFloat3(
                &view.cameraPosition
            )
        );
    for (int i = 0; i < 6; ++i)
    {
        XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYawFromVector(CAMERA_CUBEMAP_ANGLES[i]);
        
        XMMATRIX viewMatrix =
            XMMatrixInverse(
                nullptr,
                XMMatrixMultiply(
                    rotationMatrix,
                    translationMatrix
                )
            );
        XMStoreFloat4x4(&view.viewMatrix, viewMatrix);

        XMStoreFloat4x4(
            &this->viewProjectionMatrices[i],
            XMMatrixMultiply(
                viewMatrix,
                projectionMatrix
            )
        );

        context.renderer->AddView(view);
    }
}

void PointLightComponent::Render(const RenderView& view, RenderQueue& queue)
{
    if (view.type != ViewType::PRIMARY)
        return;

    TransformComponent *transform = this->GetOwner()->GetComponent<TransformComponent>();
    if (!transform)
        return;

    if (this->currentShadowIndex > 5)
        LogError("Are point lights meant to render twice? Shouldn't primary view only render once?");

    PointLightCommand command{};

    command.colour = this->colour;
    command.intensity = this->intensity;
    command.position = transform->GetRenderPosition();
    command.radius = this->radius;
    command.castShadows = this->castShadows;
    command.viewProjectionMatrix = this->viewProjectionMatrices[currentShadowIndex];

    queue.Submit(command);

    currentShadowIndex++;
}