#pragma once

#include "scene/component.hpp"

#include <d3d11.h>
#include <deque>

class TransformComponent;
class HitboxComponent;
class Texture2D;
class PlayerControllerComponent;

class SwordTrailComponent : public Component {
    RegisterComponent(SwordTrailComponent);

    ExportProperty(DirectX::XMFLOAT3, tipOffset, {0.0f, 12.0f, 0.0f});
    ExportProperty(DirectX::XMFLOAT3, baseOffset, {0.0f, 2.1f, 0.0f});

    ExportProperty(float, lifetime, 0.12f);
    ExportPropertyColor(DirectX::XMFLOAT3, tint, {1.0f, 1.0f, 1.0f});
    ExportPropertyRanged(float, emissive, "", 0.0f, 10.0f, 0.1f, 1.0f);

    struct TrailSample {
        DirectX::XMFLOAT3 tip;
        DirectX::XMFLOAT3 base;
        float time;
    };

    static constexpr uint32_t MAX_SAMPLE_COUNT = 64;

    std::deque<TrailSample> samples;

    ID3D11Buffer *vertexBuffer = nullptr;
    ID3D11Buffer *indexBuffer = nullptr;
    uint32_t indexCount = 0;

    uint32_t combo = 0;
    int prev = 0;

    std::shared_ptr<Texture2D> albedoTexture;
    std::shared_ptr<Texture2D> normalTexture;
    std::shared_ptr<Texture2D> metallicRoughnessTexture;

    TransformComponent *transform = nullptr;
    HitboxComponent *hitbox = nullptr;
    TransformComponent *viewmodelTransform = nullptr;
    PlayerControllerComponent *playerController = nullptr;

    void UploadVertices(ID3D11DeviceContext *deviceContext);

public:
    SwordTrailComponent(Entity *owner, bool isActive) : Component(owner, isActive) { UsesLateUpdate(), UsesRender(); }

private:
    void OnStart(const AppContext &context) override;
    void LateUpdate(const AppContext &context, float deltaTime) override;
    void Render(const RenderView &view, RenderQueue &queue) override;
    void OnDestroy(const AppContext &context) override;

public:
    void SetPlayerController(PlayerControllerComponent *playerController) { this->playerController = playerController; }
};