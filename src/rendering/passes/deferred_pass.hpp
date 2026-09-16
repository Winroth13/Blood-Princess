#pragma once
#include "rendering/resources/gbuffer.hpp"

#include <vector>
#include <memory>

class AssetManager;
class VertexShader;
class PixelShader;
class ComputeShader;

enum class GBufferType
{
	POSITION,
	NORMAL,
	COLOR,
	SIZE
};

class DeferredPass
{
	std::vector<GBuffer> gBuffers{};
	ID3D11Texture2D *depthStencilTexture{ nullptr };
	ID3D11DepthStencilView *depthStencilView{ nullptr };

	// TODO: Change to unique ptr and expose the load 
	// functions in asset loader without having to use the Asset Manager.
	std::shared_ptr<VertexShader> vertexShader = nullptr;
	std::shared_ptr<PixelShader> pixelShader = nullptr;
	std::shared_ptr<ComputeShader> computeShader = nullptr;

	bool Create(
		ID3D11Device* device,
		uint32_t width,
		uint32_t height
	);

public:
	DeferredPass() = default;
	DeferredPass(const DeferredPass &other) = delete;
	~DeferredPass();

	bool Initialize(
		ID3D11Device *device,
		AssetManager *assetManager,
		uint32_t width,
		uint32_t height
	);

	bool Resize(
		ID3D11Device *device,
		uint32_t width,
		uint32_t height
	);

};