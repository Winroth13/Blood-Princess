#pragma once

#include "rendering/render_view.hpp"
#include "core/logging.hpp"

#include <vector>
#include <string>

#define SafeRelease(obj) do { if (obj) (obj)->Release(); (obj) = nullptr; } while (0)

namespace RenderUtils
{
    RenderView *GetView(std::vector<RenderView> &views, ViewType type, int index = 0); 

    bool CreateConstantBuffer(ID3D11Device *device, UINT bufferSize, ID3D11Buffer **outBuffer);

	template <typename T>
	inline bool UploadConstantBuffer(ID3D11DeviceContext *deviceContext, ID3D11Buffer *buffer, const T &data) {
		if (!deviceContext) {
			LogWarn("Device context was nullptr\n");
			return false;
		}

		if (!buffer) {
			LogWarn("Buffer was nullptr\n");
			return false;
		}

		D3D11_MAPPED_SUBRESOURCE mapped;
		HRESULT result = deviceContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
		if (FAILED(result)) {
			LogWarn("Failed to map resouce\n");
			return false;
		}

		*reinterpret_cast<T *>(mapped.pData) = data;
		deviceContext->Unmap(buffer, 0);

        return true;
	}

    bool CreateStructuredBuffer(
        ID3D11Device *device,
        UINT elementSize,
        UINT elementCount,
        ID3D11Buffer **outBuffer,
        ID3D11ShaderResourceView **outSRV,
        const char *debugName
    );

    template <typename T>
    bool UploadStructuredBuffer(ID3D11DeviceContext *deviceContext, UINT elementCount, ID3D11Buffer *outBuffer, T data[]) {
        D3D11_MAPPED_SUBRESOURCE mapped;
        HRESULT result = deviceContext->Map(outBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        if (FAILED(result)) {
            LogWarn("Failed to map structured buffer\n");
            return false;
        }

        memcpy(mapped.pData, data, sizeof(T) * elementCount);
        deviceContext->Unmap(outBuffer, 0);
        
        return true;
    }

#ifdef _DEBUG

	template <typename T>
	void SetDebugName(T object, const std::string &name)
	{
		std::string finalName = "[BP] " + name;
		object->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(name), finalName.data());
	}

#else

	template <typename T>
	void SetDebugName(T object, const std::string &name) {}

#endif // _DEBUG

	namespace Impl
	{
		bool Initialize(ID3D11DeviceContext *deviceContext);
		void Shutdown();
		void BeginEvent(std::string &&name);
		void EndEvent();
	}
}

#ifdef _DEBUG

#define InitializeAnnotator(deviceContext) RenderUtils::Impl::Initialize(deviceContext)
#define ShutdownAnnotator() RenderUtils::Impl::Shutdown()
#define BeginAnnotation(name) RenderUtils::Impl::BeginEvent(name)
#define EndAnnotation() RenderUtils::Impl::EndEvent()

#else // _DEBUG

#define InitializeAnnotator(deviceContext) (void)0
#define ShhutdownAnnotator() (void)0
#define BeginAnnotation(name) (void)0
#define EndAnnoration() (void)0

#endif // _DEBUG