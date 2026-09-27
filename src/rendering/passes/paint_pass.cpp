#include "paint_pass.hpp"
#include "rendering/render_utils.hpp"
#include "assets/types/vertex_shader.hpp"
#include "assets/types/pixel_shader.hpp"
#include "assets/types/compute_shader.hpp"
#include "assets/asset_manager.hpp"
#include "rendering/shared_resources.hpp"
#include "rendering/render_data.hpp"

#include <array>


bool PaintPass::CreateInputLayout(ID3D11Device *device, VertexShader *vertexShader)
{
	LogInfo("Creating Input Layout");
	LogIndent();

	std::array<D3D11_INPUT_ELEMENT_DESC, 4> desc =
	{
		D3D11_INPUT_ELEMENT_DESC{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		D3D11_INPUT_ELEMENT_DESC{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		D3D11_INPUT_ELEMENT_DESC{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,       0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		D3D11_INPUT_ELEMENT_DESC{ "TEXCOORD",	1, DXGI_FORMAT_R32G32_FLOAT,       0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	HRESULT result = device->CreateInputLayout(
		desc.data(), static_cast<UINT>(desc.size()),
		this->vertexShader->GetByteCode().data(),
		this->vertexShader->GetByteCode().length(),
		&this->inputLayout
	);

	if (FAILED(result))
	{
		LogError("Failed to create Input Layout");
		return false;
	}

	RenderUtils::SetDebugName(this->inputLayout, DebugCategory::RENDER_PASS, "Input Layout", "Paint");

	LogUnindent();

    return true;
}

bool PaintPass::CreatePaintBuffer(ID3D11Device *device)
{
	if (!RenderUtils::CreateConstantBuffer(device, sizeof(PaintData), &this->paintBuffer, "Paint Buffer"))
		return false;

	return true;
}

bool PaintPass::CreateRasterizerState(ID3D11Device *device)
{
	D3D11_RASTERIZER_DESC desc = {};
	desc.FillMode = D3D11_FILL_SOLID;
	desc.CullMode = D3D11_CULL_NONE;
	desc.DepthClipEnable = FALSE;
	desc.ScissorEnable = FALSE;

	HRESULT result = device->CreateRasterizerState(&desc, &this->rasterizerState);

	if (FAILED(result))
	{
		LogError("DirectX failed to create Rasterizer State");
		return false;
	}

	return true;
}

bool PaintPass::CreateBlendState(ID3D11Device *device)
{
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;

	D3D11_RENDER_TARGET_BLEND_DESC &rtDesc = blendDesc.RenderTarget[0];
	rtDesc.BlendEnable = TRUE;

	// RGB: Standard alpha blending or additive accumulation
	rtDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	rtDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	rtDesc.BlendOp = D3D11_BLEND_OP_ADD;

	// Alpha: Take the HIGHER alpha between old and new (prevents fading out)
	rtDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtDesc.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	rtDesc.BlendOpAlpha = D3D11_BLEND_OP_MAX;

	rtDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HRESULT result = device->CreateBlendState(&blendDesc, &this->blendState);

	if (FAILED(result))
	{
		LogError("DirectX failed to create Blend State");
		return false;
	}

	return true;
}

void PaintPass::Render(
	ID3D11Device *device, 
	ID3D11DeviceContext *deviceContext, 
	SharedResources &sharedResources, 
	std::vector<RenderView> &views
)
{
	RenderView *view = RenderUtils::GetView(views, ViewType::PRIMARY);
	if (!view)
		return;
	
	BeginAnnotation("Paint Pass");

	D3D11_VIEWPORT viewport{};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	deviceContext->VSSetShader(this->vertexShader->GetShader(), nullptr, 0);
	deviceContext->PSSetShader(this->pixelShader->GetShader(), nullptr, 0);

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetInputLayout(this->inputLayout);

	ID3D11Buffer *perObject = sharedResources.GetPerObject();
	deviceContext->VSSetConstantBuffers(+ConstantBufferSlot::PER_OBJECT, 1, &perObject);
	deviceContext->PSSetConstantBuffers(+ConstantBufferSlot::PAINT, 1, &this->paintBuffer);

	deviceContext->RSSetState(this->rasterizerState);
	deviceContext->OMSetBlendState(this->blendState, nullptr, 0xffffffff);

	for (const PaintCommand &command : view->queue.GetPaintCommands())
	{
		PerObjectData perObjectData{};
		XMMATRIX worldMatrix = XMLoadFloat4x4(&command.worldMatrix);
		XMStoreFloat4x4(&perObjectData.worldMatrix, XMMatrixTranspose(worldMatrix));
		XMStoreFloat4x4(&perObjectData.worldInverseTransposeMatrix, XMMatrixInverse(nullptr, worldMatrix));
		RenderUtils::UploadConstantBuffer(deviceContext, sharedResources.GetPerObject(), perObjectData);

		PaintData paintData{};
		paintData.paintPosition = command.paintPosition;
		paintData.paintRadius = command.paintRadius;
		RenderUtils::UploadConstantBuffer(deviceContext, this->paintBuffer, paintData);

		ID3D11RenderTargetView *rtv = command.paintMaskRTV;
		deviceContext->OMSetRenderTargets(1, &rtv, nullptr);

		ID3D11ShaderResourceView *splatterMaskSrv = command.splatterMaskSRV;
		deviceContext->PSSetShaderResources(0, 1, &splatterMaskSrv);

		viewport.Width = static_cast<float>(command.maskWidth);
		viewport.Height = static_cast<float>(command.maskHeight);
		deviceContext->RSSetViewports(1, &viewport);

		const UINT stride = sizeof(Vertex);
		const UINT offset = 0;
		deviceContext->IASetVertexBuffers(0, 1, &command.vertexBuffer, &stride, &offset);
		deviceContext->IASetIndexBuffer(command.indexBuffer, DXGI_FORMAT_R32_UINT, 0);

		deviceContext->DrawIndexed(command.indexCount, command.startIndex, 0);
	}

	ID3D11ShaderResourceView *nullSrvs[1]{};
	deviceContext->PSSetShaderResources(0, 1, nullSrvs);

	deviceContext->VSSetShader(nullptr, nullptr, 0);
	deviceContext->PSSetShader(nullptr, nullptr, 0);

	deviceContext->RSSetState(nullptr);
	deviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);

	EndAnnotation();
}

bool PaintPass::Initialize(ID3D11Device *device, AssetManager *assetManager)
{
	LogInfo("Initializing Paint Pass...");
	LogIndent();

	this->vertexShader = assetManager->Get<VertexShader>("assets/shaders/vs_paint.cso");
	this->pixelShader = assetManager->Get<PixelShader>("assets/shaders/ps_paint.cso");

	if (!this->CreateInputLayout(device, this->vertexShader.get()))
		return false;

	if (!this->CreatePaintBuffer(device))
		return false;

	if (!this->CreateRasterizerState(device))
		return false;

	if (!this->CreateBlendState(device))
		return false;

	LogUnindent();

    return true;
}

void PaintPass::Shutdown()
{
	SafeRelease(this->blendState);
	SafeRelease(this->rasterizerState);
	SafeRelease(this->paintBuffer);
    SafeRelease(this->inputLayout);
}
