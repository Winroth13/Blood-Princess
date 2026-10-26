#include "sword_trail_component.hpp"
#include "scene/entity.hpp"
#include "components/core/transform_component.hpp"
#include "components/collisions/hit_box_component.hpp"
#include "assets/asset_manager.hpp"
#include "assets/texture.hpp"
#include "assets/types/texture_2d.hpp"
#include "assets/types/model.hpp"
#include "rendering/renderer.hpp"
#include "rendering/render_utils.hpp"
#include "rendering/render_commands.hpp"
#include "core/logging.hpp"

using namespace DirectX;

void SwordTrailComponent::OnStart(const AppContext &context) {
    this->transform = this->GetOwner()->GetComponent<TransformComponent>();
    this->hitbox = this->GetOwner()->GetComponent<HitboxComponent>();

    if (Entity *viewmodel = this->GetOwner()->GetParent())
        this->viewmodelTransform = viewmodel->GetComponent<TransformComponent>();

    if (!this->transform || !this->hitbox || !this->viewmodelTransform)
        LogWarn("SwordTrailComponent needs a transform, a hitbox and a parent with a transform");

    this->albedoTexture = context.assetManager->Get<Texture2D>("assets/images/sword_trail_2.png");
    // this->albedoTexture = context.assetManager->GetDefaultTexture(DefaultTexture::ALBEDO);
    this->normalTexture = context.assetManager->GetDefaultTexture(DefaultTexture::NORMAL);
    this->metallicRoughnessTexture = context.assetManager->GetDefaultTexture(DefaultTexture::METALLIC_ROUGHNESS);

    ID3D11Device *device = context.renderer->GetDevice();

    D3D11_BUFFER_DESC vertexDesc{};
    vertexDesc.ByteWidth = sizeof(Vertex) * MAX_SAMPLE_COUNT * 2;
    vertexDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    if (FAILED(device->CreateBuffer(&vertexDesc, nullptr, &this->vertexBuffer)))
        LogError("Failed to create sword trail vertex buffer");

    std::vector<uint32_t> indices;
    indices.reserve((MAX_SAMPLE_COUNT - 1) * 12);

    for (uint32_t i = 0; i < MAX_SAMPLE_COUNT - 1; ++i) {
        uint32_t base0 = i * 2;
        uint32_t tip0 = i * 2 + 1;
        uint32_t base1 = i * 2 + 2;
        uint32_t tip1 = i * 2 + 3;

        indices.insert(indices.end(), { base0, tip0, base1, tip0, tip1, base1 });
        indices.insert(indices.end(), { base0, base1, tip0, tip0, base1, tip1 });
    }

    D3D11_BUFFER_DESC indexDesc{};
    indexDesc.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * indices.size());
    indexDesc.Usage = D3D11_USAGE_IMMUTABLE;
    indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA indexData{};
    indexData.pSysMem = indices.data();

    if (FAILED(device->CreateBuffer(&indexDesc, &indexData, &this->indexBuffer)))
        LogError("Failed to create sword trail index buffer");
}

void SwordTrailComponent::LateUpdate(const AppContext &context, float deltaTime) {
    if (deltaTime <= 1e-5f)
        return;

    if (!this->transform || !this->hitbox || !this->viewmodelTransform)
        return;

    for (TrailSample &sample : this->samples)
        sample.time += deltaTime;

    while (!this->samples.empty() && this->samples.back().time >= this->lifetime)
        this->samples.pop_back();

    if (this->hitbox->IsActive()) {
        XMMATRIX toViewmodel = XMMatrixInverse(nullptr, this->viewmodelTransform->GetRenderMatrix());

        XMFLOAT3 tipWorld = this->transform->TransformPointRender(this->tipOffset);
        XMFLOAT3 baseWorld = this->transform->TransformPointRender(this->baseOffset);

        TrailSample sample{};
        XMStoreFloat3(&sample.tip, XMVector3TransformCoord(XMLoadFloat3(&tipWorld), toViewmodel));
        XMStoreFloat3(&sample.base, XMVector3TransformCoord(XMLoadFloat3(&baseWorld), toViewmodel));
        sample.time = 0.0f;

        this->samples.push_front(sample);

        if (this->samples.size() > MAX_SAMPLE_COUNT)
            this->samples.pop_back();
    }

    this->UploadVertices(context.renderer->GetDeviceContext());
}

void SwordTrailComponent::UploadVertices(ID3D11DeviceContext *deviceContext) {
    this->indexCount = 0;

    if (this->samples.size() < 2 || !this->vertexBuffer)
        return;

    D3D11_MAPPED_SUBRESOURCE mapped;
    if (FAILED(deviceContext->Map(this->vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
        return;

    Vertex *vertices = static_cast<Vertex *>(mapped.pData);

    for (size_t i = 0; i < this->samples.size(); ++i) {
        const TrailSample &sample = this->samples[i];
        float t = sample.time / this->lifetime;

        XMFLOAT3 base;
        XMStoreFloat3(&base, XMVectorLerp(XMLoadFloat3(&sample.base), XMLoadFloat3(&sample.tip), t));

        float u = t * 0.96f + 0.02f;

        vertices[i * 2] = Vertex(base, { 0.0f, 1.0f, 0.0f }, { u, 0.75f }, { 0.0f, 0.0f });
        vertices[i * 2 + 1] = Vertex(sample.tip, { 0.0f, 1.0f, 0.0f }, { u, 0.25f }, { 0.0f, 0.0f });

        vertices[i * 2].tangent = { 1.0f, 0.0f, 0.0f, 1.0f };
        vertices[i * 2 + 1].tangent = { 1.0f, 0.0f, 0.0f, 1.0f };
    }

    deviceContext->Unmap(this->vertexBuffer, 0);

    this->indexCount = static_cast<uint32_t>(this->samples.size() - 1) * 12;
}

void SwordTrailComponent::Render(const RenderView &view, RenderQueue &queue) {
    if (view.type != ViewType::PRIMARY || this->indexCount == 0)
        return;

    GeometryCommand command{};

    command.vertexBuffer = this->vertexBuffer;
    command.indexBuffer = this->indexBuffer;
    command.startIndex = 0;
    command.indexCount = this->indexCount;

    command.material = Material{};
    command.tint = this->tint;
    command.emissive = this->emissive;

    command.albedoSRV = this->albedoTexture->GetShaderResourceView();
    command.normalSRV = this->normalTexture->GetShaderResourceView();
    command.metallicRoughnessSRV = this->metallicRoughnessTexture->GetShaderResourceView();

    XMStoreFloat4x4(&command.worldMatrix, this->viewmodelTransform->GetRenderMatrix());

    queue.Submit(command);
}

void SwordTrailComponent::OnDestroy(const AppContext &context) {
    SafeRelease(this->vertexBuffer);
    SafeRelease(this->indexBuffer);
}