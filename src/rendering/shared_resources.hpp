#pragma once

#include <array>
#include <cstdint>
#include <d3d11.h>
#include <DirectXMath.h>
#include "render_view.hpp"

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