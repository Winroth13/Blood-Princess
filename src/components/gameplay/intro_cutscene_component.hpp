#pragma once

#include "scene/component.hpp"

#include <memory>

class Texture2D;
class AudioClip;
class UIImageComponent;
class UITextComponent;
class AudioManager;

class IntroCutsceneComponent : public Component {
    RegisterComponent(IntroCutsceneComponent);

    float time = 0.0f;

    std::shared_ptr<Texture2D> imgCuddle;
    std::shared_ptr<Texture2D> imgProne;
    std::shared_ptr<Texture2D> imgKidnap;

    std::shared_ptr<AudioClip> sfxLaugh;
    std::shared_ptr<AudioClip> sfxPurr;
    std::shared_ptr<AudioClip> sfxScream;

    UIImageComponent *imgComp1 = nullptr;
    UIImageComponent *imgComp2 = nullptr;
    UITextComponent *txtComp = nullptr;

public:
    IntroCutsceneComponent(Entity *owner, bool isActive) : Component(owner, isActive) { UsesUpdate(); }

private:
    void OnStart(const AppContext &context) override;
    void Update(const AppContext& context, float deltaTime) override;
};