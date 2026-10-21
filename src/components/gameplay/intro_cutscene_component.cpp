#include "intro_cutscene_component.hpp"
#include "assets/asset_manager.hpp"
#include "input/input.hpp"
#include "scene/entity.hpp"
#include "components/ui/ui_image_component.hpp"
#include "audio/audio_manager.hpp"
#include "scene/scene_manager.hpp"

#include <algorithm>

#undef min
#undef max

namespace {
    float Lerp(float start, float end, float t) {
        return start + (end - start) * std::clamp(t, 0.0f, 1.0f);
    }

    float Ramp(float start, float end, float t) {
        if (end <= start)
            return 0.0f;

        return std::clamp((t - start) / (end - start), 0.0f, 1.0f);
    }
}

void IntroCutsceneComponent::OnStart(const AppContext &context) {
    this->imgCuddle = context.assetManager->Get<Texture2D>("assets/images/story_1a.png");
    this->imgProne = context.assetManager->Get<Texture2D>("assets/images/story_1b_1.png");
    this->imgKidnap = context.assetManager->Get<Texture2D>("assets/images/story_1b_2.png");

    this->sfxLaugh = context.assetManager->Get<AudioClip>("assets/audio/female_a/voice_female_a_laugh_12.ogg");
    this->sfxPurr = context.assetManager->Get<AudioClip>("assets/audio/sfx/dog.ogg");
    this->sfxScream = context.assetManager->Get<AudioClip>("assets/audio/female_a/voice_female_a_hurt_pain_05.ogg");

    std::vector<Entity *> children = this->GetOwner()->GetChildren();
    this->imgComp1 = children[0]->GetComponent<UIImageComponent>();
    this->imgComp2 = children[1]->GetComponent<UIImageComponent>();

    Input::SetMouseMode(MouseMode::CAPTURED);
}

void IntroCutsceneComponent::Update(const AppContext &context, float deltaTime) {
    deltaTime = std::min(deltaTime, 0.1f);

    const float prevTime = this->time;
    this->time += deltaTime;

    auto isAtFunc = [&](float t) { return t >= prevTime && t < this->time; };
    auto isInFunc = [&](float min, float max) { return this->time >= min && this->time < max; };

    if (isInFunc(0.0f, 5.0f)) {
        this->imgComp1->SetTexture(this->imgCuddle);

        float t = Ramp(0.0f, 5.0f, this->time);
        this->imgComp1->SetOpacity(t * t);

        if (isAtFunc(0.5f)) {
            context.audioManager->PlaySoundEffect(this->sfxPurr, false, 1.5f);
        }

        if (isAtFunc(2.5f)) {
            context.audioManager->PlaySoundEffect(this->sfxLaugh, false, 0.5f);
        }
    }

    if (isAtFunc(9.0f)) {
        this->imgComp1->SetOpacity(0.0f);
        context.audioManager->PlaySoundEffect(this->sfxScream, false, 1.0f);
    }

    if (isInFunc(12.0f, 17.0f)) {
        this->imgComp1->SetTexture(this->imgProne);

        float t = Ramp(12.0f, 17.0f, this->time);
        this->imgComp1->SetOpacity(t * t);
    }

    if (isInFunc(18.0f, 23.0f)) {
        this->imgComp2->SetTexture(this->imgKidnap);

        float t = Ramp(18.0f, 23.0f, this->time);
        this->imgComp2->SetOpacity(t * t);
    }

    if (isInFunc(25.0f, 30.0f)) {
        float t = Ramp(25.0f, 30.0f, this->time);
        t = 1.0f - t * t;
        this->imgComp1->SetOpacity(t);
        this->imgComp2->SetOpacity(t);
    }

    if (isAtFunc(30.0f)) {
        context.sceneManager->RequestSceneChange("gameplay");
    }
}
