#pragma once

#include "rendering/resources/gbuffer.hpp"
#include "rendering/render_view.hpp"
#include "rendering/shared_resources.hpp"
#include "rendering/resources/backbuffer.hpp"

#include "assets/types/compute_shader.hpp"

#include "assets/types/pixel_shader.hpp"
#include "assets/types/vertex_shader.hpp"
#include "assets/types/geometry_shader.hpp"

#include <d3d11.h>
#include <memory>

class AssetManager;

class ParticlePass
{
	ID3D11Buffer *particleSystemCB = nullptr;

	std::shared_ptr<VertexShader> vertexShader = nullptr;
	std::shared_ptr<PixelShader> pixelShader = nullptr;
	std::shared_ptr<GeometryShader> geometryShader = nullptr;
	std::shared_ptr<ComputeShader> computeShader = nullptr;

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
		ID3D11DepthStencilView *depthStencilView,
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
		ID3D11DepthStencilView *depthStencilView,
		std::vector<RenderView> &views
	);

	bool Initialize(ID3D11Device *device, AssetManager *assetManager);
	void Shutdown();
};