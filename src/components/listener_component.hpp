#pragma once
#include "scene/component.hpp"
#include "components/transform_component.hpp"
#include "audio/audio_manager.hpp"
#include "scene/entity.hpp"

class ListenerComponent : public Component
{
    RegisterComponent(ListenerComponent);

    AudioManager* audioManagerRef = nullptr;

public:
    ListenerComponent(Entity *owner, bool isActive) : Component(owner, isActive) { NoLateUpdate(), NoRender(); };

private:
    void OnStart(const AppContext& context) override;
    void Update(const AppContext& context, float deltaTime) override;
};