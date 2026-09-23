#pragma once

#include <array>
#include <cstdint>
#include <d3d11.h>
#include <DirectXMath.h>
#include "render_view.hpp"

enum class ConstantBufferSlot
{
	PER_FRAME,
	PER_VIEW,
	PER_OBJECT,
	PER_MATERIAL,
	LIGHT,
	PARTICLE,
	DEBUG
};

constexpr int operator+(ConstantBufferSlot slot) noexcept {
	return static_cast<int>(slot);
}

enum class SamplerSlot 
{
	LINEAR_WRAP,
	COUNT
};

constexpr int operator+(SamplerSlot slot) noexcept {
	return static_cast<int>(slot);
}

enum class ParticleFlags : uint32_t
{
	ANIMATED = 1 << 0,
	ADDITIVE = 1 << 1,
	RESET = 1 << 2
};

constexpr uint32_t operator+(ParticleFlags slot) noexcept {
	return static_cast<uint32_t>(slot);
}

class SharedResources
{
	ID3D11Buffer *perFrameCB = nullptr;
	ID3D11Buffer *perViewCB = nullptr;
	ID3D11Buffer *perObjectCB = nullptr;
	ID3D11Buffer *perMaterialCB = nullptr;

    ID3D11Buffer *lightBufferCB = nullptr;
    ID3D11Buffer *directionalLightBufferSB = nullptr;
    ID3D11Buffer *spotLightBufferSB = nullptr;
    ID3D11Buffer *pointLightBufferSB = nullptr;

    ID3D11ShaderResourceView *directionalLightShaderResourceView = nullptr;
    ID3D11ShaderResourceView *spotLightShaderResourceView = nullptr;
    ID3D11ShaderResourceView *pointLightShaderResourceView = nullptr;

	ID3D11SamplerState *samplers[+SamplerSlot::COUNT] = {};

	bool CreateSamplers(ID3D11Device *device);

    bool UploadLightMetaData(ID3D11DeviceContext *deviceContext, const RenderView &view);
    bool UploadDirectionalLights(ID3D11DeviceContext *deviceContext, const RenderView &view);
    bool UploadSpotLights(ID3D11DeviceContext *deviceContext, const RenderView &view);
    bool UploadPointLights(ID3D11DeviceContext *deviceContext, const RenderView &view);

public:
	static constexpr int MAX_DIRECTIONAL_LIGHT_COUNT = 4;
	static constexpr int MAX_SPOT_LIGHT_COUNT = 64;
	static constexpr int MAX_POINT_LIGHT_COUNT = 64;

	SharedResources() = default;
	~SharedResources() = default;
	
	bool Initialize(ID3D11Device *device);
	void Shutdown();

    bool UploadLights(ID3D11DeviceContext *deviceContext, const RenderView &view);

	void BindSamplers(ID3D11DeviceContext *deviceContext);

	void UploadPerViewData(
		ID3D11DeviceContext *deviceContext, 
		const RenderView &view
	) const;

	void UploadPerFrameData(ID3D11DeviceContext *deviceContext, float deltaTime);

	ID3D11Buffer *GetPerFrame() const { return this->perFrameCB; }
	ID3D11Buffer *GetPerView() const { return this->perViewCB; }
	ID3D11Buffer *GetPerObject() const { return this->perObjectCB; }
	ID3D11Buffer *GetPerMaterial() const { return this->perMaterialCB; }

	ID3D11Buffer *GetLightBuffer() const { return this->lightBufferCB; }
	ID3D11ShaderResourceView *GetDirectionalLightsSRV() const { return this->directionalLightShaderResourceView; }
	ID3D11ShaderResourceView *GetSpotLightsSRV() const { return this->spotLightShaderResourceView; }
	ID3D11ShaderResourceView *GetPointLightsSRV() const { return this->pointLightShaderResourceView; }
};