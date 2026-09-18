#pragma once

#include "rendering/render_view.hpp"
#include "core/logging.hpp"

#include <vector>
#include <string>

#define SafeRelease(obj) do { if (obj) (obj)->Release(); (obj) = nullptr; } while (0)

namespace RenderUtils
{
    RenderView *GetView(std::vector<RenderView> &views, ViewType type, int index = 0); 

	template <typename T>
	inline void UploadConstantBuffer(ID3D11DeviceContext *deviceContext, ID3D11Buffer *buffer, const T &data) {
		if (!deviceContext) {
			LogWarn("Device context was nullptr\n");
			return;
		}

		if (!buffer) {
			LogWarn("Buffer was nullptr\n");
			return;
		}

		D3D11_MAPPED_SUBRESOURCE mapped;
		HRESULT result = deviceContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
		if (FAILED(result)) {
			LogWarn("Failed to map resouce\n");
			return;
		}

		*reinterpret_cast<T *>(mapped.pData) = data;
		deviceContext->Unmap(buffer, 0);
	}

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
#define ShhutdownAnnotator() RenderUtils::Impl::Shutdown()
#define BeginAnnotation(name) RenderUtils::Impl::BeginEvent(name)
#define EndAnnoration() RenderUtils::Impl::EndEvent()

#else // _DEBUG

#define InitializeAnnotator(deviceContext) (void)0
#define ShhutdownAnnotator() (void)0
#define BeginAnnotation(name) (void)0
#define EndAnnoration() (void)0

#endif // _DEBUG