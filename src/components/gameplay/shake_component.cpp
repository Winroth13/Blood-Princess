#include "shake_component.hpp"
#include "input/input.hpp"
#include "core/logging.hpp"

using namespace DirectX;

void ShakeComponent::AddTrauma(float amount)
{
	this->trauma += amount;

	if (this->trauma > 1.0f)
	{
		this->trauma = 1.0f;
	}
}

void ShakeComponent::Update(const AppContext &context, float deltaTime)
{
	this->elapsedTime += deltaTime;
	this->trauma -= this->traumaDecay * deltaTime;

	if (this->trauma < 0.0f)
	{
		this->trauma = 0.0f;
	}
	// Temporary test
	if (Input::IsKeyPressed(Key::T))
	{
		this->AddTrauma(0.6f);
		LogInfo("Shake triggered, trauma %.2f", this->trauma);
	}
}

XMFLOAT3 ShakeComponent::GetPositionOffset() const
{
	if (this->trauma <= 0.0f)
	{
		return { 0.0f, 0.0f, 0.0f };
	}

	// Squared gives a softer tail than linear: the shake drops off quickly at
	// first and settles gently instead of stopping abruptly
	const float shake = this->trauma * this->trauma;

	return {
		this->maxOffset.x * shake * this->Noise(0, this->elapsedTime),
		this->maxOffset.y * shake * this->Noise(1, this->elapsedTime),
		this->maxOffset.z * shake * this->Noise(2, this->elapsedTime)
	};
}

// Degrees, to match TransformComponent::GetWorldRotation
XMFLOAT3 ShakeComponent::GetRotationOffset() const
{
	if (this->trauma <= 0.0f)
	{
		return { 0.0f, 0.0f, 0.0f };
	}

	const float shake = this->trauma * this->trauma;

	return {
		this->maxRotation.x * shake * this->Noise(3, this->elapsedTime),
		this->maxRotation.y * shake * this->Noise(4, this->elapsedTime),
		this->maxRotation.z * shake * this->Noise(5, this->elapsedTime)
	};
}

// Continuous noise in [-1, 1], sampled along a fixed curve rather than drawn
// fresh every frame. Two samples per channel are interpolated, so the result is
// smooth and independent of frame rate: a higher frame rate samples the same
// curve more often, it does not change the curve.
float ShakeComponent::Noise(int channel, float time) const
{
	const float sampleTime = time * this->frequency;

	const int index = static_cast<int>(sampleTime);
	const float fraction = sampleTime - index;

	const float a = this->HashedValue(channel, index);
	const float b = this->HashedValue(channel, index + 1);

	// Smoothstep instead of straight interpolation, or the curve gets corners
	// at every sample point and the shake visibly ticks
	const float smooth = fraction * fraction * (3.0f - 2.0f * fraction);

	return a + (b - a) * smooth;
}

// A pure function: the same channel and index always give the same value. This
// is why RNG is not used here, since that would advance a shared generator and
// make the result depend on how often it happened to be called.
float ShakeComponent::HashedValue(int channel, int index) const
{
	unsigned int hash = static_cast<unsigned int>(index) * 1664525u;
	hash += static_cast<unsigned int>(channel) * 1013904223u;
	hash += this->noiseSeed;

	hash ^= hash >> 16;
	hash *= 2246822519u;
	hash ^= hash >> 13;
	hash *= 3266489917u;
	hash ^= hash >> 16;

	// Map the top bits to [0, 1), then to [-1, 1]
	const float normalized = (hash >> 8) / static_cast<float>(1u << 24);
	return normalized * 2.0f - 1.0f;
}