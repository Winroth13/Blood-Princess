#pragma once

#include "scene/component.hpp"
#include "DirectXMath.h"

class ShakeComponent : public Component
{
    RegisterComponent(ShakeComponent);

    ExportPropertyCategory("Decay");
    ExportPropertyRanged(float, traumaDecay, "", 0.1f, 10.0f, 0.1f, 1.5f);
    ExportPropertyRanged(float, frequency, "Hz", 1.0f, 60.0f, 1.0f, 25.0f);

    ExportPropertyCategory("Amplitude");
    ExportPropertyRanged(DirectX::XMFLOAT3, maxOffset, "m", 0.0f, 1.0f, 0.01f, { 0.1f, 0.1f, 0.0f });
    ExportPropertyRanged(DirectX::XMFLOAT3, maxRotation, "deg", 0.0f, 10.0f, 0.1f, { 1.5f, 1.5f, 2.0f });

    float trauma = 0.0f;
    float elapsedTime = 0.0f;
    unsigned int noiseSeed = 0;

public:
    ShakeComponent(Entity *owner, bool isActive) : Component(owner, isActive) { UsesUpdate(); }

private:
    void Update(const AppContext &context, float deltaTime) override;

    // Continuous noise in [-1, 1], sampled along a fixed curve
    float Noise(int channel, float time) const;
    float HashedValue(int channel, int index) const;

public:
    void AddTrauma(float amount);
    float GetTrauma() const { return this->trauma; }

    DirectX::XMFLOAT3 GetPositionOffset() const;

    // Degrees, to match TransformComponent::GetWorldRotation
    DirectX::XMFLOAT3 GetRotationOffset() const;

    const DirectX::XMFLOAT3 &GetMaxOffset() const { return this->maxOffset; }
    void SetMaxOffset(const DirectX::XMFLOAT3 &maxOffset) { this->maxOffset = maxOffset; }

    const DirectX::XMFLOAT3 &GetMaxRotation() const { return this->maxRotation; }
    void SetMaxRotation(const DirectX::XMFLOAT3 &maxRotation) { this->maxRotation = maxRotation; }

    float GetTraumaDecay() const { return this->traumaDecay; }
    void SetTraumaDecay(float traumaDecay) { this->traumaDecay = traumaDecay; }

    float GetFrequency() const { return this->frequency; }
    void SetFrequency(float frequency) { this->frequency = frequency; }

    unsigned int GetNoiseSeed() const { return this->noiseSeed; }
    void SetNoiseSeed(unsigned int noiseSeed) { this->noiseSeed = noiseSeed; }
};