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
    ExportProperty(bool, isAnimated, false);
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
    ExportProperty(bool, isAdditive, false);
    ExportPropertyColor(XMFLOAT3, startTint, { 1.0f, 1.0f, 1.0f });
    ExportPropertyColor(XMFLOAT3, endTint, { 1.0f, 1.0f, 1.0f });

    ID3D11Buffer *particleSystemSB = nullptr;
    ID3D11ShaderResourceView *particleSystemSRV = nullptr;
    ID3D11UnorderedAccessView *particleSystemUAV = nullptr;

    bool shouldReset = true;

public:
    ParticleSystemComponent(Entity *owner, bool isActive) : Component(owner, isActive) { NoUpdate(); }

private:
    void OnStart(const AppContext &context) override;
    void Update(const AppContext &context, float deltaTime) override {}
    void LateUpdate(const AppContext &context, float deltaTime) override {}
    void Render(const RenderView &view, RenderQueue &queue) override;
    void OnDestroy(const AppContext &context) override;

    bool CreateParticleBuffer(ID3D11Device* device);

public:
    void SetCount(const uint32_t count) { this->count = count; }
    void SetLifeTime(const float lifeTime) { this->lifeTime = lifeTime; }
    void SetSpawnRadius(const float spawnRadius) { this->spawnRadius = spawnRadius; }
    void SetAnimationSpeed(const float animationSpeed) { this->animationSpeed = animationSpeed; }
    void SetAtlasSize(const XMFLOAT2 atlasSize) { this->atlasSize = atlasSize; }
    void SetAtlasTexture(std::shared_ptr<Texture2D> atlasTexture) { this->atlasTexture = atlasTexture; }
    void SetVelocity(const XMFLOAT3 velocity) { this->velocity = velocity; }
    void SetAcceleration(const XMFLOAT3 acceleration) { this->acceleration = acceleration; }
    void SetStartTint(const XMFLOAT3 startTint) { this->startTint = startTint; }
    void SetEndTint(const XMFLOAT3 endTint) { this->endTint = endTint; }
    void SetStartScale(const float startScale) { this->startScale = startScale; }
    void SetEndScale(const float endScale) { this->endScale = endScale; }

    void SetIsAnimate(const bool isAnimated) { this->isAnimated = isAnimated; }
    void SetIsAdditive(const bool isAdditive) { this->isAdditive = isAdditive; }
};