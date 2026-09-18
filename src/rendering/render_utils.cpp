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

        void BeginEvent(std::string &&name)
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