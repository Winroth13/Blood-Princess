#include "shared_resources.hpp"
#include "core/logging.hpp"
#include "rendering/render_utils.hpp"

using namespace DirectX;

bool SharedResources::CreatePerViewBuffer(ID3D11Device *device) 
{
	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.ByteWidth = sizeof(PerViewData);
	desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA data = {};
	data.pSysMem = &desc;

	HRESULT hr = device->CreateBuffer(&desc, &data, &this->perView);

	if (FAILED(hr))
	{
		LogError("Failed to create Per View Buffer");
		return false;
	}

	return true;
}

bool SharedResources::CreatePerObjectBuffer(ID3D11Device *device) 
{
	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.ByteWidth = sizeof(PerObjectData);
	desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA data = {};
	data.pSysMem = &desc;

	HRESULT hr = device->CreateBuffer(&desc, &data, &this->perObject);

	if (FAILED(hr))
	{
		LogError("Failed to create Per Object Buffer");
		return false;
	}

	return true;
}

bool SharedResources::CreatePerMaterialBuffer(ID3D11Device *device) 
{
	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.ByteWidth = sizeof(PerMaterialData);
	desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA data = {};
	data.pSysMem = &desc;

	HRESULT hr = device->CreateBuffer(&desc, &data, &this->perMaterial);

	if (FAILED(hr))
	{
		LogError("Failed to create Per Material Buffer");
		return false;
	}

	return true;
}

bool SharedResources::CreateSamplers(ID3D11Device *device) 
{
	D3D11_SAMPLER_DESC desc{};
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.MaxAnisotropy = 1;
	desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	desc.MinLOD = 0;
	desc.MaxLOD = D3D11_FLOAT32_MAX;

	HRESULT result = device->CreateSamplerState(&desc, &this->samplers[+SamplerSlot::LINEAR_WRAP]);
	if (FAILED(result)) {
		LogError("Failed to create Linear Wrap Sampler");
		return false;
	}

	return true;
}

bool SharedResources::Initialize(ID3D11Device* device)
{
	LogInfo("Creating Shared Resources...");
	LogIndent();

	if (!this->CreatePerViewBuffer(device))
		return false;
	
	if (!this->CreatePerObjectBuffer(device))
		return false;
	
	if (!this->CreatePerMaterialBuffer(device))
		return false;
	
	if (!this->CreateSamplers(device))
		return false;

	LogUnindent();

	return true;
}

void SharedResources::Shutdown()
{
	LogInfo("Shutting down Shared Resources...");
	LogIndent();

	SafeRelease(this->perView);
	SafeRelease(this->perObject);
	SafeRelease(this->perMaterial);

	for (size_t i = 0; i < +SamplerSlot::COUNT; ++i)
		SafeRelease(this->samplers[i]);
	
	LogUnindent();
}

void SharedResources::BindSamplers(ID3D11DeviceContext *deviceContext)
{
	deviceContext->VSSetSamplers(0, +SamplerSlot::COUNT, this->samplers);
	deviceContext->PSSetSamplers(0, +SamplerSlot::COUNT, this->samplers);
	deviceContext->CSSetSamplers(0, +SamplerSlot::COUNT, this->samplers);
	deviceContext->DSSetSamplers(0, +SamplerSlot::COUNT, this->samplers);
}

void SharedResources::UploadPerViewData(ID3D11DeviceContext *deviceContext, const RenderView &view) const {
	XMMATRIX viewMatrix = XMLoadFloat4x4(&view.viewMatrix);
	XMMATRIX projectionMatrix = XMLoadFloat4x4(&view.projectionMatrix);
	XMMATRIX viewProjectionMatrix = XMMatrixMultiply(viewMatrix, projectionMatrix);

	PerViewData data{};
	XMStoreFloat4x4(&data.viewMatrix, XMMatrixTranspose(viewMatrix));
	XMStoreFloat4x4(&data.invViewMatrix, XMMatrixTranspose(XMMatrixInverse(nullptr, viewMatrix)));
	XMStoreFloat4x4(&data.projectionMatrix, XMMatrixTranspose(projectionMatrix));
	XMStoreFloat4x4(&data.invProjectionMatrix, XMMatrixTranspose(XMMatrixInverse(nullptr, projectionMatrix)));
	XMStoreFloat4x4(&data.viewProjectionMatrix, XMMatrixTranspose(viewProjectionMatrix));
	XMStoreFloat4x4(&data.invViewProjectionMatrix, XMMatrixTranspose(XMMatrixInverse(nullptr, viewProjectionMatrix)));
	data.cameraPosition = view.cameraPosition;

	RenderUtils::UploadConstantBuffer(deviceContext, this->perView, data);
}