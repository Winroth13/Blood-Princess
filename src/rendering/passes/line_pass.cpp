#include "line_pass.hpp"
#include "rendering/render_utils.hpp"
#include "core/logging.hpp"
#include "assets/asset_manager.hpp"
#include "rendering/resources/backbuffer.hpp"
#include "rendering/shared_resources.hpp"

constexpr size_t MAX_POINTS = 2048;

bool LinePass::CreateInputLayout(ID3D11Device *device, VertexShader *vertexShader)
{
	LogInfo("Creating Input Layout");
	LogIndent();

	D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
	{
		{ "POSITION",	0,	DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,	D3D11_INPUT_PER_VERTEX_DATA,	0 },
		{ "COLOR",		0,	DXGI_FORMAT_R32G32B32_FLOAT,    0, 12,	D3D11_INPUT_PER_VERTEX_DATA,	0 },
	};

	HRESULT result = device->CreateInputLayout(
		layoutDesc, 2,
		vertexShader->GetByteCode().data(),
		vertexShader->GetByteCode().length(),
		&this->inputLayout
	);

	if (FAILED(result))
	{
		LogError("Failed to create Input Layout");
		return false;
	}

	RenderUtils::SetDebugName(this->inputLayout, DebugCategory::RENDER_PASS, "Input Layout", "Lines");

	LogUnindent();

	return true;
}

bool LinePass::CreateVertexBuffer(ID3D11Device *device)
{
	D3D11_BUFFER_DESC desc = {};
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.ByteWidth = sizeof(LineVertex) * MAX_POINTS;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	HRESULT result = device->CreateBuffer(&desc, nullptr, &this->vertexBuffer);

	if (FAILED(result))
	{
		LogError("DirectX failed to create Vertex Buffer");
		return false;
	}

    RenderUtils::SetDebugName(this->vertexBuffer, DebugCategory::RENDER_PASS, "VB", "Lines");

	return true;
}

bool LinePass::CreateDisableDepthResources(ID3D11Device *device)
{
	HRESULT result;

	{
		D3D11_DEPTH_STENCIL_DESC desc = {};
		desc.DepthEnable = false;

		result = device->CreateDepthStencilState(&desc, &this->disableDepthStencilState);

		if (FAILED(result))
		{
			LogError("DirectX failed to create Depth Stencil");
			return false;
		}

        RenderUtils::SetDebugName(this->disableDepthStencilState, DebugCategory::RENDER_PASS, "DSS", "Lines");
	}

	{
		D3D11_RASTERIZER_DESC desc = {};
		desc.CullMode = D3D11_CULL_FRONT;
		desc.FillMode = D3D11_FILL_SOLID;
		desc.DepthClipEnable = false;
		
		result = device->CreateRasterizerState(&desc, &this->disableDepthRasterizerState);

		if (FAILED(result))
		{
			LogError("DirectX failed to create Rasterizer State");
			return false;
		}

        RenderUtils::SetDebugName(this->disableDepthRasterizerState, DebugCategory::RENDER_PASS, "RS", "Lines");
	}

	return true;
}

void LinePass::Render(
	ID3D11Device *device,
	ID3D11DeviceContext *deviceContext,
	SharedResources &sharedResources,
	BackBuffer &backBuffer,
	std::vector<RenderView> &views
)
{
	BeginAnnotation("Line Pass");

	RenderView *view = RenderUtils::GetView(views, ViewType::PRIMARY);
	if (!view)
	{
		LogWarn("Could not find a primary view to render to");
		return;
	}

	sharedResources.UploadPerViewData(deviceContext, *view);

	ID3D11RenderTargetView *rtv = backBuffer.GetRenderTargetView();
	deviceContext->OMSetRenderTargets(1, &rtv, backBuffer.GetDepthStencilView());

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	deviceContext->IASetInputLayout(this->inputLayout);

	const UINT stride = sizeof(LineVertex);
	const UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, &this->vertexBuffer, &stride, &offset);

	deviceContext->VSSetShader(this->vertexShader->GetShader(), nullptr, 0);
	deviceContext->PSSetShader(this->pixelShader->GetShader(), nullptr, 0);

	ID3D11Buffer *perView = sharedResources.GetPerView();
	deviceContext->VSSetConstantBuffers(+ConstantBufferSlot::PER_VIEW, 1, &perView);

	D3D11_VIEWPORT viewport{};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(backBuffer.GetWidth());
	viewport.Height = static_cast<float>(backBuffer.GetHeight());
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	deviceContext->RSSetViewports(1, &viewport);

	for (const LineCommand &command : view->queue.GetLineCommands())
	{
		if (command.points.size() > MAX_POINTS)
		{
			LogWarn("Line command was given with %zu points. Maxiumum is %zu", command.points.size(), MAX_POINTS);
			continue;
		}

		if (!command.enableDepth)
		{
			deviceContext->RSSetState(this->disableDepthRasterizerState);
			deviceContext->OMSetDepthStencilState(this->disableDepthStencilState, 0);
		}
		else
		{
			deviceContext->RSSetState(nullptr);
			deviceContext->OMSetDepthStencilState(nullptr, 0);
		}

		D3D11_MAPPED_SUBRESOURCE mapped;
		deviceContext->Map(this->vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

		LineVertex *vertices = static_cast<LineVertex*>(mapped.pData);
		memcpy(vertices, command.points.data(), sizeof(LineVertex) * command.points.size());

		deviceContext->Unmap(this->vertexBuffer, 0);
		deviceContext->Draw(static_cast<UINT>(command.points.size()), 0);
	}

	deviceContext->RSSetState(nullptr);
	deviceContext->OMSetDepthStencilState(nullptr, 0);

	deviceContext->IASetInputLayout(nullptr);

	deviceContext->VSSetShader(nullptr, nullptr, 0);
	deviceContext->PSSetShader(nullptr, nullptr, 0);

	EndAnnotation();
}

bool LinePass::Initialize(ID3D11Device *device, AssetManager *assetManager)
{
	LogInfo("Initializing Line Pass...");
	LogIndent();

	this->vertexShader = assetManager->Get<VertexShader>("assets/shaders/vs_line.cso");
	this->pixelShader = assetManager->Get<PixelShader>("assets/shaders/ps_line.cso");

	if (!this->CreateInputLayout(device, this->vertexShader.get()))
		return false;

	if (!this->CreateVertexBuffer(device))
		return false;

	if (!this->CreateDisableDepthResources(device))
		return false;

	LogUnindent();

	return true;
}

void LinePass::Shutdown()
{
	SafeRelease(this->disableDepthRasterizerState);
	SafeRelease(this->disableDepthStencilState);
	SafeRelease(this->vertexBuffer);
	SafeRelease(this->inputLayout);
}
