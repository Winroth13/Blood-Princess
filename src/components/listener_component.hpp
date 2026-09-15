#pragma once
#include "scene/component.hpp"
#include "components/transform_component.hpp"
#include "audio/audio_manager.hpp"
#include "scene/entity.hpp"

class ListenerComponent : public Component
{
    RegisterComponent(ListenerComponent);

public:
    ListenerComponent(Entity* owner, bool isActive) : Component(owner, isActive) {};

    void OnStart(const AppContext& context) override;
    void Update(const AppContext& context, float deltaTime) override;
    void Render(const RenderView& view, RenderQueue& queue) override {};
    void OnDestroy(const AppContext& context) override {};

private:
    AudioManager* audioManagerRef = nullptr;
};

