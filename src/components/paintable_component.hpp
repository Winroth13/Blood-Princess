#pragma once
#include "scene/entity.hpp"
#include "scene/component.hpp"
#include "events/signal.hpp"

struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;
struct ID3D11RenderTargetView;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11UnorderedAccessView;

class PaintableComponent : public Component
{
    RegisterComponent(PaintableComponent);

    ExportProperty(std::shared_ptr<Texture2D>, paintMask, nullptr);

    std::vector<ID3D11Texture2D *> maskTextures{};
    std::vector<ID3D11ShaderResourceView *> maskSRVs{};
    std::vector<ID3D11RenderTargetView *> maskRTVs{};

public:
    PaintableComponent(Entity *owner, bool isActive) : Component(owner, isActive) {};

private:
    void OnStart(const AppContext &context) override;
    void Update(const AppContext &context, float deltaTime) override {};
    void LateUpdate(const AppContext &context, float deltaTime) override {}
    void Render(const RenderView &view, RenderQueue &queue) override;
    void OnDestroy(const AppContext &context) override;

    void CreatePaintMask(ID3D11Device *device, ID3D11DeviceContext *deviceContext, uint32_t index);

public:
    void SetPaintMask(std::shared_ptr<Texture2D> paintMask) const { this->paintMask; }

    ID3D11ShaderResourceView *GetMaskSRV(uint32_t index) const { return this->maskSRVs[index]; }
    ID3D11RenderTargetView *GetMaskRTV(uint32_t index) const { return this->maskRTVs[index]; }
};