#include "paintable_component.hpp"
#include "core/logging.hpp"
#include "rendering/render_utils.hpp"
#include "rendering/renderer.hpp"
#include "components/model_component.hpp"
#include "components/transform_component.hpp"
#include "assets/types/texture_2d.hpp"
#include "input/input.hpp"
#include "core/rng.hpp"

#include <d3d11.h>

void PaintableComponent::OnStart(const AppContext &context)
{
	ModelComponent *modelComponent = this->GetOwner()->GetComponent<ModelComponent>();

	if (!modelComponent)
		return;
	
	auto model = modelComponent->GetModel();
	if (!model)
		return;

	const auto &meshes = model->GetMeshes();

	maskTextures.resize(meshes.size(), nullptr);
	maskSRVs.resize(meshes.size(), nullptr);
	maskRTVs.resize(meshes.size(), nullptr);

	for (uint32_t i = 0; i < meshes.size(); ++i)
	{
		this->CreatePaintMask(
			context.renderer->GetDevice(), 
			context.renderer->GetDeviceContext(), 
			i
		);
	}
}

void PaintableComponent::Render(const RenderView &view, RenderQueue &queue)
{
	if (!Input::IsKeyDown(Key::SPACE))
		return;

	if (view.type != ViewType::PRIMARY)
		return;

	TransformComponent* transformComponent = this->GetOwner()->GetComponent<TransformComponent>();

	if (!transformComponent)
		return;

	ModelComponent *modelComponent = this->GetOwner()->GetComponent<ModelComponent>();

	if (!modelComponent)
		return;

	uint32_t meshIndex = 0;
	const auto &meshes = modelComponent->GetModel()->GetMeshes();
	for (const auto &mesh : meshes)
	{
		ID3D11RenderTargetView *rtv = this->maskRTVs[meshIndex];
		ID3D11ShaderResourceView *srv = this->maskSRVs[meshIndex];

		const auto &subMeshes = mesh->GetSubMeshes();
		for (const auto &subMesh : subMeshes)
		{
			PaintCommand command = {};

			command.vertexBuffer = mesh->GetVertexBuffer();
			command.indexBuffer = mesh->GetIndexBuffer();

			command.startIndex = subMesh.startIndex;
			command.indexCount = subMesh.numIndicies;

			XMStoreFloat4x4(&command.worldMatrix, transformComponent->GetRenderMatrix());

			command.paintPosition = { 0,0,0 };
			command.paintRadius = RNG::Float(0.2f, 0.9f);
			command.maskWidth = this->paintMaskWidth;
			command.maskHeight = this->paintMaskHeight;

			command.paintMaskRTV = rtv;
			command.paintMaskSRV = srv;
			command.splatterMaskSRV = this->paintMask->GetShaderResourceView();

			queue.Submit(command);
		}
	}
}

void PaintableComponent::OnDestroy(const AppContext &context)
{
	for (size_t i = 0; i < this->maskTextures.size(); ++i)
	{
		SafeRelease(this->maskSRVs[i]);
		SafeRelease(this->maskRTVs[i]);
		SafeRelease(this->maskTextures[i]);
	}
}

void PaintableComponent::CreatePaintMask(ID3D11Device *device, ID3D11DeviceContext *deviceContext, uint32_t index)
{
	HRESULT result{};

	{
		D3D11_TEXTURE2D_DESC desc = {};
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		desc.MipLevels = 0;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.Width = static_cast<UINT>(this->paintMaskWidth);
		desc.Height = static_cast<UINT>(this->paintMaskHeight);
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		result = device->CreateTexture2D(&desc, nullptr, &this->maskTextures[index]);

		if (FAILED(result))
		{
			LogError("Failed to create Mask Texture");
			return;
		}

		RenderUtils::SetDebugName(this->maskTextures[index], DebugCategory::TEXTURE, "Paint Mask Texture #" + std::to_string(index));
	}

	// RTV
	result = device->CreateRenderTargetView(
		this->maskTextures[index],
		nullptr,
		&this->maskRTVs[index]
	);

	if (FAILED(result))
	{
		LogError("Failed to create Mask RTV");
		return;
	}

	RenderUtils::SetDebugName(this->maskRTVs[index], DebugCategory::TEXTURE, "Paint Mask Texture", "RTV #" + std::to_string(index));

	// SRV
	result = device->CreateShaderResourceView(
		this->maskTextures[index],
		nullptr,
		&this->maskSRVs[index]
	);

	if (FAILED(result))
	{
		LogError("Failed to create Mask SRV");
		return;
	}

	RenderUtils::SetDebugName(this->maskSRVs[index], DebugCategory::TEXTURE, "Paint Mask Texture", "SRV #" + std::to_string(index));

	// Fill Black
	float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	deviceContext->ClearRenderTargetView(this->maskRTVs[index], color);
}
