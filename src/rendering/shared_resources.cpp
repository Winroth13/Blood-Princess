#include "shared_resources.hpp"
#include "core/logging.hpp"
#include "rendering/render_utils.hpp"

using namespace DirectX;

bool SharedResources::Initialize()
{
	LogInfo("Creating Shared Resources...");
	LogIndent();

	/* Per  */
	{
		D3D11_BUFFER_DESC desc = {};
	}

	LogUnindent();

	return true;
}

void SharedResources::Shutdown()
{

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