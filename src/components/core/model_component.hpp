#pragma once
#include "scene/component.hpp"

#include <d3d11.h>

#include <memory>

class Model;

class ModelComponent : public Component 
{
    RegisterComponent(ModelComponent)

    ExportProperty(std::shared_ptr<Model>, model, nullptr);
    ExportPropertyColor(DirectX::XMFLOAT3, tint, { 1.0f, 1.0f, 1.0f });
    ExportProperty(bool, isHsvAdjusted, false);

public:
    ModelComponent(Entity *owner, bool isActive) : Component(owner, isActive) { UsesRender(); }

private:
    void Render(const RenderView &view, RenderQueue &queue) override;

public:
    std::shared_ptr<Model> GetModel() const { return this->model; }
    void SetModel(std::shared_ptr<Model> model);

    DirectX::XMFLOAT3 GetTint() const { return this->tint; }
    void SetTint(DirectX::XMFLOAT3 tint) { this->tint = tint; }

    bool GetIsHsvAdjusted() const { return this->isHsvAdjusted; }
    void SetIsHsvAdjusted(bool isHsvAdjusted) { this->isHsvAdjusted = isHsvAdjusted; }

    bool GetWorldBounds(DirectX::BoundingBox &outBounds) override;
};