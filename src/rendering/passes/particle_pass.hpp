#pragma once

#include "rendering/resources/gbuffer.hpp"
#include "rendering/render_view.hpp"
#include "rendering/shared_resources.hpp"
#include "rendering/resources/backbuffer.hpp"

#include <d3d11.h>

class ParticlePass
{
	ID3D11Buffer *particleSystemCB = nullptr;

	void RenderParticlePass(
		ID3D11Device *device,
		ID3D11DeviceContext *deviceContext,
		SharedResources &sharedResources,
		std::vector<RenderView> &views
	);
	void RenderDrawPass(
		ID3D11Device *device,
		ID3D11DeviceContext *deviceContext,
		SharedResources &sharedResources,
		ID3D11RenderTargetView *renderTargetView,
		std::vector<RenderView> &views
	);

public:
	ParticlePass() = default;
	~ParticlePass() = default;

	void Render(
		ID3D11Device *device,
		ID3D11DeviceContext *deviceContext,
		SharedResources &sharedResources,
		ID3D11RenderTargetView *renderTargetView,
		std::vector<RenderView> &views
	);

	bool Initialize(ID3D11Device *device);
	void Shutdown();
};