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

    bool CreateConstantBuffer(ID3D11Device *device, UINT bufferSize, ID3D11Buffer **outBuffer) {
        D3D11_BUFFER_DESC desc = {};
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;
        desc.ByteWidth = bufferSize;
        desc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA data{};
        data.pSysMem = &desc;

        HRESULT result = device->CreateBuffer(&desc, &data, outBuffer);
        if (FAILED(result))
        {
            LogWarn("Failed to create Constant Buffer");
            return false;
        }

        return true;
    }

    bool CreateStructuredBuffer(
        ID3D11Device *device, 
        UINT elementSize, 
        UINT elementCount, 
        ID3D11Buffer **outBuffer, 
        ID3D11ShaderResourceView **outSRV, 
        const char *debugName
    ) {
        D3D11_BUFFER_DESC desc{};
        desc.ByteWidth = elementSize * elementCount;
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        desc.StructureByteStride = elementSize;

        HRESULT result = device->CreateBuffer(&desc, nullptr, outBuffer);
        if (FAILED(result)) {
            LogError("Failed to create Structured Buffer '%s'", debugName);
            return false;
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        srvDesc.Buffer.FirstElement = 0;
        srvDesc.Buffer.NumElements = elementCount;

        result = device->CreateShaderResourceView(*outBuffer, &srvDesc, outSRV);
        if (FAILED(result)) {
            LogError("Failed to create SRV for '%s'", debugName);
            return false;
        }

        return true;
    }

    namespace Impl
    {
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
}