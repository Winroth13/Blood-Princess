#pragma once
#include "rendering/shared_resources.hpp"

class AssetManager;
class VertexShader;
class PixelShader;

class ShadowPass
{
	ID3D11Texture2D *depthStencilTexture = nullptr;
	ID3D11DepthStencilView *depthStencilView = nullptr;
	ID3D11ShaderResourceView *depthStencilSRV = nullptr;

	ID3D11InputLayout *inputLayout = nullptr;

	// TODO: Change to unique ptr and expose the load 
	// functions in asset loader without having to use the Asset Manager.
	std::shared_ptr<VertexShader> vertexShader = nullptr;
	std::shared_ptr<PixelShader> pixelShader = nullptr;
	std::shared_ptr<PixelShader> pixelShaderLinear = nullptr;

	std::array<ID3D11Texture2D*, SharedResources::MAX_DIRECTIONAL_LIGHT_COUNT> directionalShadows;
	std::array<ID3D11Texture2D*, SharedResources::MAX_SPOT_LIGHT_COUNT> spotShadows;
	std::array<ID3D11Texture2D**, SharedResources::MAX_POINT_LIGHT_COUNT> pointShadows;

	uint32_t size = 0;

	bool Create(
		ID3D11Device *device,
		uint32_t size
	);

	bool CreateInputLayout(ID3D11Device *device);

public:
	ShadowPass() = default;
	~ShadowPass() = default;

	ShadowPass(const ShadowPass &other) = delete;
	ShadowPass &operator=(const ShadowPass &other) = delete;

	bool Initialize(
		ID3D11Device *device,
		AssetManager *assetManager,
		uint32_t size
	);

	void Shutdown();

	bool Resize(
		ID3D11Device *device,
		uint32_t size
	);

	void Render(
		ID3D11Device *device,
		ID3D11DeviceContext *deviceContext,
		SharedResources &sharedResources,
		std::vector<RenderView> &views
	);
};