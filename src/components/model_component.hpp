#pragma once
#include "scene/component.hpp"

#include <memory>

class Model;

class ModelComponent : public Component 
{
    std::shared_ptr<Model> model = nullptr;

public:
    ModelComponent(Entity *owner, bool isActive) : Component(owner, isActive) { NoUpdate(); }

    void OnStart(const AppContext &context) override {}
    void Update(const AppContext &context, float deltaTime) override {}
    void Render(const RenderView &view, RenderQueue &queue) override;
    void OnDestroy(const AppContext &context) override {}

    void SetModel(std::shared_ptr<Model> model);
};