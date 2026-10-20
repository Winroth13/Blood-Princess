#pragma once

#include <array>
#include <cstdint>
#include <d3d11.h>
#include <DirectXMath.h>
#include "render_view.hpp"

enum class ConstantBufferSlot : int
{
	// Shared
	PER_FRAME = 0,
	PER_VIEW = 1,
	PER_OBJECT = 2,
	PER_MATERIAL = 3,

	LIGHT = 4,
	PARTICLE = 5,
	POST_PROCESSING = 6,
	ENVIRONMENT = 7,

	// Unique
	DEBUG = 8,

	PAINT = 8,

    SKINNING = 9
};

constexpr int operator+(ConstantBufferSlot slot) noexcept {
	return static_cast<int>(slot);
}

enum class SamplerSlot 
{
	LINEAR_WRAP,
	SHADOW,
    LINEAR_CLAMP,

	COUNT
};

constexpr int operator+(SamplerSlot slot) noexcept {
	return static_cast<int>(slot);
}

enum class ParticleFlags : uint32_t
{
	ANIMATED = 1 << 0,
	ADDITIVE = 1 << 1,
	RESET = 1 << 2,
	GLOBAL = 1 << 3,
	BURST = 1 << 4,
	SPREAD = 1 << 5
};

constexpr uint32_t operator+(ParticleFlags slot) noexcept {
	return static_cast<uint32_t>(slot);
}

enum class MaterialFlags : uint32_t
{
	HSV_ADJUSTED = 1 << 0
};

constexpr uint32_t operator+(MaterialFlags slot) noexcept {
	return static_cast<uint32_t>(slot);
}

enum class EnvironmentFlags : uint32_t
{
	HAS_SKYBOX = 1 << 0
};

constexpr uint32_t operator+(EnvironmentFlags slot) noexcept {
	return static_cast<uint32_t>(slot);
}

class SharedResources
{
	ID3D11Buffer *perFrameCB = nullptr;
	ID3D11Buffer *perViewCB = nullptr;
	ID3D11Buffer *perObjectCB = nullptr;
	ID3D11Buffer *perMaterialCB = nullptr;
	ID3D11Buffer *skinningCB = nullptr;
	ID3D11Buffer *environmentBufferCB = nullptr;

    ID3D11Buffer *lightBufferCB = nullptr;
    ID3D11Buffer *directionalLightBufferSB = nullptr;
    ID3D11Buffer *spotLightBufferSB = nullptr;
    ID3D11Buffer *pointLightBufferSB = nullptr;

    ID3D11ShaderResourceView *directionalLightShaderResourceView = nullptr;
    ID3D11ShaderResourceView *spotLightShaderResourceView = nullptr;
    ID3D11ShaderResourceView *pointLightShaderResourceView = nullptr;

	static constexpr int MAX_DIRECTIONAL_LIGHT_COUNT = 4;
	static constexpr int MAX_SPOT_LIGHT_COUNT = 64;
	static constexpr int MAX_POINT_LIGHT_COUNT = 64;

	ID3D11Texture2D *directionalShadowsDepthTextureArray = nullptr; // DSDTA
	ID3D11Texture2D *spotShadowsDepthTextureArray = nullptr;
	ID3D11Texture2D *pointShadowsTextureCubeArray = nullptr;

	std::array<ID3D11DepthStencilView *, MAX_DIRECTIONAL_LIGHT_COUNT> directionalShadowsDepthStencilViews{ nullptr };
	std::array<ID3D11DepthStencilView *, MAX_SPOT_LIGHT_COUNT> spotShadowsDepthStencilViews{ nullptr };
	std::array<ID3D11RenderTargetView *, MAX_POINT_LIGHT_COUNT * 6> pointShadowsRenderTargetViews{ nullptr };

	ID3D11ShaderResourceView *directionalShadowsShaderResourceView = nullptr;
	ID3D11ShaderResourceView *spotShadowsShaderResourceView = nullptr;
	ID3D11ShaderResourceView *pointShadowsShaderResourceView = nullptr;

	ID3D11SamplerState *samplers[+SamplerSlot::COUNT] = {};

	bool CreateSamplers(ID3D11Device *device);
	bool CreateShadowMaps(ID3D11Device *device, uint32_t shadowSize, uint32_t shadowSizeDirectional);

    bool UploadLightMetaData(ID3D11DeviceContext *deviceContext, const RenderView &view);
    bool UploadDirectionalLights(ID3D11DeviceContext *deviceContext, const RenderView &view);
    bool UploadSpotLights(ID3D11DeviceContext *deviceContext, const RenderView &view);
    bool UploadPointLights(ID3D11DeviceContext *deviceContext, const RenderView &view);

public:
	SharedResources() = default;
	~SharedResources() = default;
	
	bool Initialize(ID3D11Device *device, uint32_t shadowSize, uint32_t shadowSizeDirectional);
	void Shutdown();

    bool UploadLights(ID3D11DeviceContext *deviceContext, const RenderView &view);

	void BindSamplers(ID3D11DeviceContext *deviceContext);

	void UploadPerViewData(
		ID3D11DeviceContext *deviceContext, 
		const RenderView &view
	) const;

	void UploadPerFrameData(ID3D11DeviceContext *deviceContext, float totalTime, float deltaTime);

    void UploadSkinMatrices(ID3D11DeviceContext *deviceContext, const XMFLOAT4X4 *skinMatrices, uint32_t skinMatricesCount);

	void UploadEnvironmentData(ID3D11DeviceContext *deviceContext, EnvironmentCommand environment);

	ID3D11Buffer *GetPerFrame() const { return this->perFrameCB; }
	ID3D11Buffer *GetPerView() const { return this->perViewCB; }
	ID3D11Buffer *GetPerObject() const { return this->perObjectCB; }
	ID3D11Buffer *GetPerMaterial() const { return this->perMaterialCB; }
	ID3D11Buffer *GetSkinning() const { return this->skinningCB; }
	ID3D11Buffer *GetEnvironment() const { return this->environmentBufferCB; }

	ID3D11Buffer *GetLightBuffer() const { return this->lightBufferCB; }
	ID3D11ShaderResourceView *GetDirectionalLightsSRV() const { return this->directionalLightShaderResourceView; }
	ID3D11ShaderResourceView *GetSpotLightsSRV() const { return this->spotLightShaderResourceView; }
	ID3D11ShaderResourceView *GetPointLightsSRV() const { return this->pointLightShaderResourceView; }

	ID3D11Texture2D *GetDirectionalLightsDepthTextureArray() const { return this->directionalShadowsDepthTextureArray; }
	ID3D11Texture2D *GetSpotLightsDepthTextureArray() const { return this->spotShadowsDepthTextureArray; }
	ID3D11Texture2D *GetPointShadowsTextureCubeArray() const { return this->pointShadowsTextureCubeArray; }

	ID3D11DepthStencilView *GetDirectionalShadowDSV(int index) const { return directionalShadowsDepthStencilViews[index]; }
	ID3D11DepthStencilView *GetSpotShadowDSV(int index) const { return spotShadowsDepthStencilViews[index]; }
	ID3D11RenderTargetView *GetPointShadowRTV(int index) const { return pointShadowsRenderTargetViews[index]; }

	ID3D11ShaderResourceView *GetDirectionalShadowsSRV() const { return this->directionalShadowsShaderResourceView; }
	ID3D11ShaderResourceView *GetSpotShadowsSRV() const { return this->spotShadowsShaderResourceView; }
	ID3D11ShaderResourceView *GetPointShadowsSRV() const { return this->pointShadowsShaderResourceView; }
};