#pragma once

#include <array>
#include <cstdint>
#include <d3d11.h>
#include <DirectXMath.h>
#include "render_view.hpp"

//struct PerFrameBuffer
//{
//	DirectX::XMFLOAT3 ambientColor;
//	uint32_t numDirectionalLights;
//	uint32_t numPointLights;
//	uint32_t numSpotLights;
//	uint32_t flags;
//	float pad0;
//	std::array<uint32_t, 2> screenDimensions;
//	DirectX::XMFLOAT2 pad1;
//};

struct PerViewData
{
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 invViewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;
	DirectX::XMFLOAT4X4 invProjectionMatrix;
	DirectX::XMFLOAT4X4 viewProjectionMatrix;
	DirectX::XMFLOAT4X4 invViewProjectionMatrix;
	DirectX::XMFLOAT3   cameraPosition;
	float               pad0;
};

struct PerObjectData
{
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 worldInverseTransposeMatrix;
};

struct PerMaterialData
{
	DirectX::XMFLOAT3 ambientCoefficient;
	float phongExponent;
	DirectX::XMFLOAT3 diffuseCoefficient;
	float reflectiveness;
	DirectX::XMFLOAT3 specularCoefficient;
	uint32_t materialFlags;
	float maxTessFactor;
	float maxTessDistance;
	float minTessDistance;
	float dispStrength;
};

enum class SamplerSlot {
	LINEAR_WRAP,
	COUNT
};

constexpr int operator+(SamplerSlot slot) noexcept {
	return static_cast<int>(slot);
}

class SharedResources
{
	ID3D11Buffer *perView{ nullptr };
	ID3D11Buffer *perObject{ nullptr };
	ID3D11Buffer *perMaterial{ nullptr };

	ID3D11SamplerState *samplers[+SamplerSlot::COUNT] = {};

	bool CreatePerViewBuffer(ID3D11Device *device);
	bool CreatePerObjectBuffer(ID3D11Device *device);
	bool CreatePerMaterialBuffer(ID3D11Device *device);

	bool CreateSamplers(ID3D11Device *device);

public:
	SharedResources() = default;
	~SharedResources() = default;
	
	bool Initialize(ID3D11Device *device);
	void Shutdown();

	void BindSamplers(ID3D11DeviceContext *deviceContext);
	void UploadPerViewData(
		ID3D11DeviceContext *deviceContext, 
		const RenderView &view
	) const;

	ID3D11Buffer *GetPerView() const { return this->perView; }
	ID3D11Buffer *GetPerObject() const { return this->perObject; }
	ID3D11Buffer *GetPerMaterial() const { return this->perMaterial; }
};