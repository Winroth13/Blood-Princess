#pragma once
#include "rendering/resources/gbuffer.hpp"
#include "rendering/render_view.hpp"
#include "rendering/shared_resources.hpp"
#include "rendering/resources/backbuffer.hpp"

#include <vector>
#include <memory>

#include <d3d11.h>

class AssetManager;
class VertexShader;
class PixelShader;
class ComputeShader;

enum class GBufferType
{
	ALBEDO_METALLIC,	// Albedo | Metallic
	NORMAL_ROUGHNESS,	// Encoded Normal | Roughness

    OUTPUT,             // Not a Gbuffer but I don't care

    EMISSIVE,

	COUNT
};

static constexpr int operator+(GBufferType val) noexcept {
	return static_cast<int>(val);
}

class DeferredPass
{
public:
    struct OutputTextures {
        ID3D11ShaderResourceView *albedoMetallic;
        ID3D11ShaderResourceView *normalRoughness;
        ID3D11ShaderResourceView *output;
        ID3D11ShaderResourceView *emissive;
    };

private:
	std::vector<GBuffer> gBuffers{};

	ID3D11InputLayout *inputLayout = nullptr;
    ID3D11InputLayout *skinnedInputLayout = nullptr;

	// TODO: Change to unique ptr and expose the load 
	// functions in asset loader without having to use the Asset Manager.
	std::shared_ptr<VertexShader> vertexShader = nullptr;
    std::shared_ptr<VertexShader> skinnedVertexShader = nullptr;
	std::shared_ptr<PixelShader> pixelShader = nullptr;
	std::shared_ptr<ComputeShader> computeShader = nullptr;

	uint32_t width = 0;
	uint32_t height = 0;

	bool Create(
		ID3D11Device* device,
		uint32_t width,
		uint32_t height
	);

	bool CreateInputLayout(ID3D11Device *device);

	GBuffer &GetGBuffer(GBufferType type) { return this->gBuffers[+type]; }

	void RenderGeometryPass(
		ID3D11Device *device,
		ID3D11DeviceContext *deviceContext,
		SharedResources &sharedResources,
		BackBuffer &backBuffer,
		std::vector<RenderView> &views
	);

	void RenderLightingPass(
        ID3D11Device *device,
        ID3D11DeviceContext *deviceContext,
        SharedResources &sharedResources,
		BackBuffer &backBuffer,
        std::vector<RenderView> &views
	);

public:
	DeferredPass() = default;
	~DeferredPass() = default;

	DeferredPass(const DeferredPass &other) = delete;
    DeferredPass &operator=(const DeferredPass &other) = delete;

	bool Initialize(
		ID3D11Device *device,
		AssetManager *assetManager,
		uint32_t width,
		uint32_t height
	);

	void Shutdown();

	bool Resize(
		ID3D11Device *device,
		uint32_t width,
		uint32_t height
	);

	void Render(
        ID3D11Device *device,
        ID3D11DeviceContext *deviceContext,
        SharedResources &sharedResources,
        BackBuffer &backBuffer,
        std::vector<RenderView> &views
	);

    OutputTextures GetOutputTextures();
};