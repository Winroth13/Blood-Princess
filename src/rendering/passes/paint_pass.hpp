#pragma once
#include "rendering/lines/line_vertex.hpp"
#include "rendering/render_view.hpp"

#include <memory>
#include <vector>

struct ID3D11InputLayout;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Buffer;
struct ID3D11RasterizerState;
struct ID3D11BlendState;

class VertexShader;
class PixelShader;
class ComputeShader;
class AssetManager;
class SharedResources;
class BackBuffer;

/**
*	Splatoon Painting Effect inspired by this amazing YT video.
*	https://www.youtube.com/watch?v=YUWfHX_ZNCw
*/

class PaintPass
{
	ID3D11InputLayout *inputLayout = nullptr;
	ID3D11Buffer *paintBuffer = nullptr;
	ID3D11RasterizerState *rasterizerState = nullptr;
	ID3D11BlendState *blendState = nullptr;

	std::shared_ptr<VertexShader> vertexShader = nullptr;
	std::shared_ptr<PixelShader> pixelShader = nullptr;

	bool CreateInputLayout(ID3D11Device *device, VertexShader *vertexShader);
	bool CreatePaintBuffer(ID3D11Device *device);
	bool CreateRasterizerState(ID3D11Device *device);
	bool CreateBlendState(ID3D11Device *device);

public:
	PaintPass() = default;
	~PaintPass() = default;

	void Render(
		ID3D11Device *device,
		ID3D11DeviceContext *deviceContext,
		SharedResources &sharedResources,
		std::vector<RenderView> &views
	);

	bool Initialize(ID3D11Device *device, AssetManager *assetManager);
	void Shutdown();
};