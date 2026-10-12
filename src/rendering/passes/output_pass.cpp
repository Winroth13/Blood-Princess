#include "output_pass.hpp"
#include "core/logging.hpp"
#include "assets/asset_manager.hpp"
#include "assets/types/compute_shader.hpp"
#include "rendering/render_utils.hpp"
#include "core/debug.hpp"

bool OutputPass::Initialize(
    ID3D11Device *device,
    AssetManager *assetManager,
    uint32_t width,
    uint32_t height
) {
    LogInfo("Initializing Deferred Pass...");
    LogIndent();

    if (!RenderUtils::CreateConstantBuffer(device, sizeof(DebugViewData), &this->debugCB, "Debug View"))
        return false;

    if (!RenderUtils::CreateConstantBuffer(device, sizeof(PostProcessingData), &this->postProcessingCB, "Post Processing"))
        return false;

    this->noiseTexture = assetManager->Get<Texture2D>("assets/images/noise.png");

    this->computeShader = assetManager->Get<ComputeShader>("assets/shaders/cs_output.cso");

    this->width = width;
    this->height = height;

    LogUnindent();
    return true;
}

void OutputPass::Shutdown() {
    LogInfo("Shutting down Output Pass");

    SafeRelease(this->debugCB);
}

bool OutputPass::Resize(
    ID3D11Device *device,
    uint32_t width,
    uint32_t height
) {
    this->width = width;
    this->height = height;

    return true;
}

void OutputPass::Render(
    ID3D11Device *device,
    ID3D11DeviceContext *deviceContext,
    SharedResources &sharedResources,
    DeferredPass &deferredPass,
    BackBuffer &backBuffer,
    std::vector<RenderView> &views
) {
    BeginAnnotation("Output Pass");

    RenderView *view = RenderUtils::GetView(views, ViewType::PRIMARY);
    if (!view) {
        LogWarn("Could not find a primary view to render to");
        return;
    }

    sharedResources.UploadPerViewData(deviceContext, *view);

    DebugViewData debugViewData{};
    debugViewData.debugView = +this->debugView;
    RenderUtils::UploadConstantBuffer(deviceContext, this->debugCB, debugViewData);

    PostProcessingData postProcessingData{};
    postProcessingData.speedLineColour = Debug::GetSetting("speedLines.colour", DirectX::XMFLOAT4{ 1.0f, 1.0f, 1.0f, 1.0f });
    postProcessingData.speedLineCount = Debug::GetSetting("speedLines.count", 2.0f);
    postProcessingData.speedLineDensity = Debug::GetSetting("speedLines.density", 0.4f);
    postProcessingData.speedLineFaloff = Debug::GetSetting("speedLines.falloff", 0.25f);
    postProcessingData.speedLineMaskSize = Debug::GetSetting("speedLines.maskSize", 0.1f);
    postProcessingData.speedLineMaskEdge = Debug::GetSetting("speedLines.maskEdge", 0.5f);
    postProcessingData.speedLineAnimationSpeed = Debug::GetSetting("speedLines.animationSpeed", 10.0f);

    RenderUtils::UploadConstantBuffer(deviceContext, this->postProcessingCB, postProcessingData);

    DeferredPass::OutputTextures deferredOutput = deferredPass.GetOutputTextures();

    ID3D11UnorderedAccessView *backBufferUAV = backBuffer.GetRenderTargetUAV();
    deviceContext->CSSetUnorderedAccessViews(0, 1, &backBufferUAV, nullptr);

    std::array<ID3D11ShaderResourceView *, 5> srvs = {
        deferredOutput.albedoMetallic,
        deferredOutput.normalRoughness,
        backBuffer.GetDepthStencilSRV(),
        deferredOutput.output,
        this->noiseTexture->GetShaderResourceView()
    };
    deviceContext->CSSetShaderResources(0, static_cast<UINT>(srvs.size()), srvs.data());

    deviceContext->CSSetShader(this->computeShader->GetShader(), nullptr, 0);

    ID3D11Buffer *perFrame = sharedResources.GetPerFrame();
    deviceContext->CSSetConstantBuffers(+ConstantBufferSlot::PER_FRAME, 1, &perFrame);
    ID3D11Buffer *perView = sharedResources.GetPerView();
    deviceContext->CSSetConstantBuffers(+ConstantBufferSlot::PER_VIEW, 1, &perView);

    deviceContext->CSSetConstantBuffers(+ConstantBufferSlot::DEBUG, 1, &this->debugCB);
    deviceContext->CSSetConstantBuffers(+ConstantBufferSlot::POST_PROCESSING, 1, &this->postProcessingCB);

    const UINT groupsX = (this->width + 7) / 8;
    const UINT groupsY = (this->height + 7) / 8;
    deviceContext->Dispatch(groupsX, groupsY, 1);

    ID3D11ShaderResourceView *nullSrvs[srvs.size()] = {};
    deviceContext->CSSetShaderResources(0, static_cast<UINT>(srvs.size()), nullSrvs);

    ID3D11UnorderedAccessView *nullUav = nullptr;
    deviceContext->CSSetUnorderedAccessViews(0, 1, &nullUav, nullptr);

    deviceContext->CSSetShader(nullptr, nullptr, 0);

    EndAnnotation();
}