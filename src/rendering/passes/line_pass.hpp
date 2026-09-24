#pragma once
#include "rendering/lines/line_vertex.hpp"
#include "rendering/render_view.hpp"

#include <memory>
#include <vector>

struct ID3D11InputLayout;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Buffer;

class VertexShader;
class PixelShader;
class AssetManager;
class SharedResources;
class BackBuffer;

class LinePass
{
	ID3D11InputLayout *inputLayout = nullptr;
	ID3D11Buffer *vertexBuffer = nullptr;

	std::shared_ptr<VertexShader> vertexShader = nullptr;
	std::shared_ptr<PixelShader> pixelShader = nullptr;

	bool CreateInputLayout(ID3D11Device *device, VertexShader* vertexShader);
	bool CreateVertexBuffer(ID3D11Device *device);

public:
	LinePass() = default;
	~LinePass() = default;

	void Render(
		ID3D11Device *device,
		ID3D11DeviceContext *deviceContext,
		SharedResources &sharedResources,
		BackBuffer &backBuffer,
		std::vector<RenderView> &views
	);

	bool Initialize(ID3D11Device *device, AssetManager *assetManager);
	void Shutdown();
};