#pragma once

#include "scene/component.hpp"
#include "DirectXMath.h"

class ParticleSystemComponent : public Component
{
    RegisterComponent(ParticleSystemComponent);

    ExportPropertyCategory("General");
    ExportProperty(int, count, 20);
    ExportPropertyRanged(float, lifeTime, "s", 0.0f, 0.0f, 0.1f, 1.0f);
    ExportPropertyRanged(float, spawnRadius, "m", 0.0f, 0.0f, 0.1f, 1.0f);

    ExportPropertyCategory("Animation");
    ExportPropertyRanged(float, animationSpeed, "", 0.0f, 0.0f, 0.1f, 1.0f);
    ExportPropertyRanged(XMFLOAT2, atlasSize, "", 0.0f, 0.0f, 0.1f, {1.0f, 1.0f});
    ExportProperty(std::shared_ptr<Texture2D>, atlasTexture, nullptr);

    ExportPropertyCategory("Velocity");
    ExportPropertyRanged(XMFLOAT3, velocity, "m/s", 0.0f, 0.0f, 0.1f, {0.0f, 0.2f, 0.0f});
    ExportPropertyRanged(XMFLOAT3, acceleration, "m/s^2", 0.0f, 0.0f, 0.1f, { 0.0f, 1.0f, 0.0f });

    ExportPropertyCategory("Scale");
    ExportPropertyRanged(float, startScale, "", 0.0f, 0.0f, 0.1f, 0.0f);
    ExportPropertyRanged(float, endScale, "", 0.0f, 0.0f, 0.1f, 1.0f);

    ExportPropertyCategory("Color");
    ExportPropertyColor(XMFLOAT3, startTint, { 1.0f, 1.0f, 1.0f });
    ExportPropertyColor(XMFLOAT3, endTint, { 1.0f, 0.0f, 0.0f });

    ID3D11Buffer *particleSystemSB = nullptr;
    ID3D11ShaderResourceView *particleSystemSRV = nullptr;
    ID3D11UnorderedAccessView *particleSystemUAV = nullptr;

public:
    ParticleSystemComponent(Entity *owner, bool isActive) : Component(owner, isActive) { NoUpdate(); }

private:
    void OnStart(const AppContext &context) override;
    void Update(const AppContext &context, float deltaTime) override {}
    void Render(const RenderView &view, RenderQueue &queue) override;
    void OnDestroy(const AppContext &context) override;

    bool CreateParticleBuffer(ID3D11Device* device);

public:
};