#include "output_pass.hpp"
#include "core/logging.hpp"
#include "assets/asset_manager.hpp"
#include "assets/types/compute_shader.hpp"
#include "rendering/render_utils.hpp"

bool OutputPass::Initialize(
    ID3D11Device *device,
    AssetManager *assetManager,
    uint32_t width,
    uint32_t height
) {
    LogInfo("Initializing Deferred Pass...");
    LogIndent();

    this->computeShader = assetManager->Get<ComputeShader>("assets/shaders/cs_output.cso");

    this->width = width;
    this->height = height;

    LogUnindent();
    return true;
}

void OutputPass::Shutdown() {

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
    RenderView *view = RenderUtils::GetView(views, ViewType::PRIMARY);
    if (!view) {
        LogWarn("Could not find a primary view to render to");
        return;
    }

    sharedResources.UploadPerViewData(deviceContext, *view);

    DeferredPass::OutputTextures deferredOutput = deferredPass.GetOutputTextures();

    ID3D11UnorderedAccessView *backBufferUAV = backBuffer.GetUAV();
    deviceContext->CSSetUnorderedAccessViews(0, 1, &backBufferUAV, nullptr);

    std::array<ID3D11ShaderResourceView *, 4> srvs = {
        deferredOutput.albedoMetallic,
        deferredOutput.normalRoughness,
        deferredOutput.depth,
        deferredOutput.output
    };
    deviceContext->CSSetShaderResources(0, static_cast<UINT>(srvs.size()), srvs.data());

    deviceContext->CSSetShader(this->computeShader->GetShader(), nullptr, 0);

    ID3D11Buffer *perView = sharedResources.GetPerView();
    deviceContext->CSSetConstantBuffers(0, 1, &perView);

    const UINT groupsX = (this->width + 7) / 8;
    const UINT groupsY = (this->height + 7) / 8;
    deviceContext->Dispatch(groupsX, groupsY, 1);

    ID3D11ShaderResourceView *nullSrvs[srvs.size()] = {};
    deviceContext->CSSetShaderResources(0, static_cast<UINT>(srvs.size()), nullSrvs);

    ID3D11UnorderedAccessView *nullUav = nullptr;
    deviceContext->CSSetUnorderedAccessViews(0, 1, &nullUav, nullptr);

    deviceContext->CSSetShader(nullptr, nullptr, 0);
}