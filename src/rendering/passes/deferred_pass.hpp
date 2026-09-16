#pragma once
#include "rendering/resources/gbuffer.hpp"

#include <vector>

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

public:
	DeferredPass() = default;
	DeferredPass(const DeferredPass &other) = delete;
	~DeferredPass();

	bool Create(
		ID3D11Device* device,
		uint32_t width,
		uint32_t height
	);

	bool Resize(
		ID3D11Device *device,
		uint32_t width,
		uint32_t height
	);

};