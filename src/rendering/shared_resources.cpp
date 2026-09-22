#include "shared_resources.hpp"
#include "core/logging.hpp"
#include "rendering/render_utils.hpp"
#include "rendering/render_data.hpp"

#include <cmath>

using namespace DirectX;

static inline XMFLOAT3 operator*(const XMFLOAT3 &v, float s) 
{
    return 
    {
        v.x * s,
        v.y * s,
        v.z * s
    };
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

    RenderUtils::SetDebugName(this->samplers[+SamplerSlot::LINEAR_WRAP], DebugCategory::CORE, "Sampler", "Linear-Wrap");

	return true;
}

bool SharedResources::UploadLightMetaData(ID3D11DeviceContext *deviceContext, const RenderView &view) {
    LightData data{};

    data.ambientColour = {0.0f, 0.0f, 0.0f};
    const auto &alc = view.queue.ambientLightCommand;
    if (alc.has_value())
        data.ambientColour = alc->colour * alc->intensity;

    data.directionalLightCount = static_cast<int>(view.queue.directionalLightCommands.size());
    data.spotLightCount = static_cast<int>(view.queue.spotLightCommands.size());
    data.pointLightCount = static_cast<int>(view.queue.pointLightCommands.size());

    if (!RenderUtils::UploadConstantBuffer(deviceContext, this->lightBufferCB, data))
        return false;

    return true;
}

bool SharedResources::UploadDirectionalLights(ID3D11DeviceContext *deviceContext, const RenderView &view) {
    int shadowIndex = 0;
    std::vector<DirectionalLightData> directionalLightData;
    directionalLightData.reserve(MAX_DIRECTIONAL_LIGHT_COUNT);
    for (const DirectionalLightCommand &dlc : view.queue.directionalLightCommands) {
        DirectionalLightData data{};

        data.direction = dlc.direction;
        data.color = dlc.colour * dlc.intensity;

        if (dlc.castShadows) {
            data.castShadows = 1;
            XMStoreFloat4x4(&data.viewProjectionMatrix, XMMatrixTranspose(XMLoadFloat4x4(&dlc.viewProjectionMatrix)));
            data.shadowArrayIndex = shadowIndex++;
        }
        else {
            data.castShadows = 0;
            data.shadowArrayIndex = -1;
        }

        directionalLightData.push_back(data);
    }

    if (!RenderUtils::UploadStructuredBuffer(deviceContext, static_cast<UINT>(directionalLightData.size()), this->directionalLightBufferSB, directionalLightData.data()))
        return false;

    return true;
}

bool SharedResources::UploadSpotLights(ID3D11DeviceContext *deviceContext, const RenderView &view) {
    int shadowIndex = 0;
    std::vector<SpotLightData> spotLightData;
    spotLightData.reserve(MAX_SPOT_LIGHT_COUNT);
    for (const SpotLightCommand &slc : view.queue.spotLightCommands) {
        SpotLightData data{};

        data.position = slc.position;
        data.range = slc.range;
        data.direction = slc.direction;
        data.innerAngleCos = cosf(slc.innerAngle);
        data.color = slc.colour * slc.intensity;
        data.outerAngleCos = cosf(slc.outerAngle);

        if (slc.castShadows) {
            data.castShadows = 1;
            XMStoreFloat4x4(&data.viewProjectionMatrix, XMMatrixTranspose(XMLoadFloat4x4(&slc.viewProjectionMatrix)));
            data.shadowArrayIndex = shadowIndex++;

        }
        else {
            data.castShadows = 0;
            data.shadowArrayIndex = -1;
        }

        spotLightData.push_back(data);
    }

    if (!RenderUtils::UploadStructuredBuffer(deviceContext, static_cast<UINT>(spotLightData.size()), this->spotLightBufferSB, spotLightData.data()))
        return false;

    return true;
}

bool SharedResources::UploadPointLights(ID3D11DeviceContext *deviceContext, const RenderView &view) {
    int shadowIndex = 0;
    std::vector<PointLightData> pointLightData;
    pointLightData.reserve(MAX_POINT_LIGHT_COUNT);
    for (const PointLightCommand &plc : view.queue.pointLightCommands) {
        PointLightData data{};

        data.position = plc.position;
        data.color = plc.colour * plc.intensity;
        data.radius = plc.radius;

        if (plc.castShadows) {
            data.castShadows = 1;
            data.shadowArrayIndex = shadowIndex++;
        }
        else {
            data.castShadows = 0;
            data.shadowArrayIndex = -1;
        }

        pointLightData.push_back(data);
    }

    if (!RenderUtils::UploadStructuredBuffer(deviceContext, static_cast<UINT>(pointLightData.size()), this->pointLightBufferSB, pointLightData.data()))
        return false;

    return true;
}


bool SharedResources::Initialize(ID3D11Device* device)
{
	LogInfo("Creating Shared Resources...");
	LogIndent();

    if (!RenderUtils::CreateConstantBuffer(device, sizeof(PerViewData), &this->perViewCB, "Per View"))
        return false;

    if (!RenderUtils::CreateConstantBuffer(device, sizeof(PerObjectData), &this->perObjectCB, "Per Obejct"))
        return false;

    if (!RenderUtils::CreateConstantBuffer(device, sizeof(PerMaterialData), &this->perMaterialCB, "Per Material"))
        return false;

    if (!RenderUtils::CreateConstantBuffer(device, sizeof(LightData), &this->lightBufferCB, "Light Buffer"))
        return false;

    if (!RenderUtils::CreateStructuredBuffer(
        device,
        sizeof(DirectionalLightData),
        MAX_DIRECTIONAL_LIGHT_COUNT,
        &this->directionalLightBufferSB,
        &this->directionalLightShaderResourceView,
        "Directional Light")
    ) {
        return false;
    }

    if (!RenderUtils::CreateStructuredBuffer(
        device,
        sizeof(SpotLightData),
        MAX_SPOT_LIGHT_COUNT,
        &this->spotLightBufferSB,
        &this->spotLightShaderResourceView,
        "Spot Light")
    ) {
        return false;
    }

    if (!RenderUtils::CreateStructuredBuffer(
        device,
        sizeof(PointLightData),
        MAX_POINT_LIGHT_COUNT,
        &this->pointLightBufferSB,
        &this->pointLightShaderResourceView,
        "Point Light")
    ) {
        return false;
    }

    if (!this->CreateSamplers(device))
        return false;

	LogUnindent();

	return true;
}

void SharedResources::Shutdown()
{
	LogInfo("Shutting down Shared Resources...");
	LogIndent();

	SafeRelease(this->perViewCB);
	SafeRelease(this->perObjectCB);
	SafeRelease(this->perMaterialCB);

    SafeRelease(this->directionalLightShaderResourceView);
    SafeRelease(this->spotLightShaderResourceView);
    SafeRelease(this->pointLightShaderResourceView);
    SafeRelease(this->lightBufferCB);
    SafeRelease(this->directionalLightBufferSB);
    SafeRelease(this->spotLightBufferSB);
    SafeRelease(this->pointLightBufferSB);

	for (size_t i = 0; i < +SamplerSlot::COUNT; ++i)
		SafeRelease(this->samplers[i]);
	
	LogUnindent();
}

bool SharedResources::UploadLights(ID3D11DeviceContext *deviceContext, const RenderView &view) {
    if (!this->UploadLightMetaData(deviceContext, view))
        return false;

    if (!this->UploadDirectionalLights(deviceContext, view))
        return false;

    if (!this->UploadSpotLights(deviceContext, view))
        return false;

    if (!this->UploadPointLights(deviceContext, view))
        return false;

    return true;
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
    data.farPlane = view.farPlane;
    data.nearPlane = view.nearPlane;

	RenderUtils::UploadConstantBuffer(deviceContext, this->perViewCB, data);
}