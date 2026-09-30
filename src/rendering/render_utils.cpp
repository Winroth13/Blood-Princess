#include "render_utils.hpp"
#include "core/logging.hpp"

#include <d3d11_1.h>

namespace RenderUtils
{
    RenderView *GetView(std::vector<RenderView> &views, ViewType type, int index) {
        for (RenderView &view : views)
            if (view.type == type && view.index == index)
                return &view;

        return nullptr;
    }

    bool CreateConstantBuffer(
        ID3D11Device *device,
        UINT bufferSize,
        ID3D11Buffer **outBuffer,
        const std::string &debugName
    ) {
        D3D11_BUFFER_DESC desc = {};
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;
        desc.ByteWidth = bufferSize;
        desc.StructureByteStride = 0;

        HRESULT result = device->CreateBuffer(&desc, nullptr, outBuffer);
        if (FAILED(result))
        {
            LogWarn("Failed to create Constant Buffer '%s'", debugName.data());
            return false;
        }

        RenderUtils::SetDebugName(*outBuffer, DebugCategory::BUFFER, "CB", debugName);

        return true;
    }

    bool CreateStructuredBuffer(
        ID3D11Device *device, 
        UINT elementSize, 
        UINT elementCount, 
        ID3D11Buffer **outBuffer, 
        ID3D11ShaderResourceView **outSRV,
        ID3D11UnorderedAccessView **outUAV,
        uint32_t flags,
        const std::string &debugName
    ) {
        D3D11_USAGE usageFlags = D3D11_USAGE_DYNAMIC;

        // If we wan't a UAV we need the usage to be default since that allows for read/write on GPU
        if ((flags & D3D11_BIND_UNORDERED_ACCESS) == D3D11_BIND_UNORDERED_ACCESS)
            usageFlags = D3D11_USAGE_DEFAULT;

        D3D11_BUFFER_DESC desc{};
        desc.ByteWidth = elementSize * elementCount;
        desc.Usage = usageFlags;
        desc.BindFlags = flags;
        desc.CPUAccessFlags = ((flags & D3D11_BIND_UNORDERED_ACCESS) == D3D11_BIND_UNORDERED_ACCESS) ? 0 : D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        desc.StructureByteStride = elementSize;

        HRESULT result = device->CreateBuffer(&desc, nullptr, outBuffer);
        if (FAILED(result)) {
            LogError("Failed to create Structured Buffer '%s'", debugName.data());
            return false;
        }

        RenderUtils::SetDebugName(*outBuffer, DebugCategory::BUFFER, "SB", debugName);

        if ((flags & D3D11_BIND_SHADER_RESOURCE) == D3D11_BIND_SHADER_RESOURCE)
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
            srvDesc.Format = DXGI_FORMAT_UNKNOWN;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
            srvDesc.Buffer.FirstElement = 0;
            srvDesc.Buffer.NumElements = elementCount;

            result = device->CreateShaderResourceView(*outBuffer, &srvDesc, outSRV);
            if (FAILED(result)) {
                LogError("Failed to create SRV for '%s'", debugName.data());
                return false;
            }

            RenderUtils::SetDebugName(*outSRV, DebugCategory::BUFFER, "SB", debugName, "SRV");
        }

        if ((flags & D3D11_BIND_UNORDERED_ACCESS) == D3D11_BIND_UNORDERED_ACCESS)
        {
            D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
            uavDesc.Format = DXGI_FORMAT_UNKNOWN;
            uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
            uavDesc.Buffer.FirstElement = 0;
            uavDesc.Buffer.NumElements = elementCount;
            uavDesc.Buffer.Flags = 0;

            result = device->CreateUnorderedAccessView(*outBuffer, &uavDesc, outUAV);
            if (FAILED(result)) {
                LogError("Failed to create UAV for '%s'", debugName.data());
                return false;
            }

            RenderUtils::SetDebugName(*outUAV, DebugCategory::BUFFER, "SB", debugName, "UAV");
        }

        return true;
    }

#ifdef _DEBUG

    namespace Impl
    {
        std::string categorySymbols[7]
        {
            "🟥", "🟦", "🟧", "🟨", "🟩", "🟪", "🟫" // Happy pride!
        };

        ID3DUserDefinedAnnotation *annotator;

        bool Initialize(ID3D11DeviceContext *deviceContext)
        {
            HRESULT hr = deviceContext->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), (void **)&annotator);

            if (FAILED(hr))
            {
                LogError("Failed to create annotator");
                return false;
            }

            return true;
        }

        void Shutdown()
        {
            SafeRelease(annotator);
        }

        void BeginEvent(const std::string &&name)
        {
            wchar_t wideBuffer[32];
            size_t dummy = 0;
            mbstowcs_s(&dummy, wideBuffer, name.data(), _TRUNCATE);

            annotator->BeginEvent(wideBuffer);
        }

        void EndEvent()
        {
            annotator->EndEvent();
        }
    }

#else

    namespace Impl {}

#endif // _DEBUG
}