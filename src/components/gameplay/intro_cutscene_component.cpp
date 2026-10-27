#include "intro_cutscene_component.hpp"
#include "assets/asset_manager.hpp"
#include "input/input.hpp"
#include "input/action.hpp"
#include "scene/entity.hpp"
#include "components/ui/ui_image_component.hpp"
#include "components/ui/ui_text_component.hpp"
#include "audio/audio_manager.hpp"
#include "scene/scene_manager.hpp"
#include "components/core/transform_component.hpp"
#include "components/audio/music_component.hpp"

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

void IntroCutsceneComponent::CutsceneIntro(const AppContext &context, float prevTime) {
    auto img1 = this->imgComps[0];
    auto img2 = this->imgComps[1];
    auto txt1 = this->txtComps[0];

    if (Input::IsKeyPressed(Key::SPACE) || Input::IsGamepadButtonPressed(GamepadButton::SOUTH)) {
        if (txt1->IsActive())
            context.sceneManager->RequestSceneChange("gameplay");
        else
            txt1->SetActive(true);
    }

    auto isAtFunc = [&](float t) { return t >= prevTime && t < this->time; };
    auto isInFunc = [&](float min, float max) { return this->time >= min && this->time < max; };

    if (isInFunc(0.0f, 5.0f)) {
        img1->SetTexture(this->imgCuddle);

        float t = Ramp(0.0f, 5.0f, this->time);
        img1->SetOpacity(t * t);

        if (isAtFunc(0.5f)) {
            context.audioManager->PlaySoundEffect(this->sfxPurr, false, 1.5f);
        }

        if (isAtFunc(2.5f)) {
            context.audioManager->PlaySoundEffect(this->sfxLaugh, false, 0.5f);
        }
    }

    if (isAtFunc(9.0f)) {
        img1->SetOpacity(0.0f);
        context.audioManager->PlaySoundEffect(this->sfxBonk, false, 1.0f);
    }

    if (isAtFunc(9.5f)) {
        context.audioManager->PlaySoundEffect(this->sfxScream, false, 1.0f);
    }

    if (isInFunc(12.0f, 17.0f)) {
        img1->SetTexture(this->imgProne);

        float t = Ramp(12.0f, 17.0f, this->time);
        img1->SetOpacity(t * t);
    }

    if (isAtFunc(12.0f)) {
        Entity *wind = context.sceneManager->GetCurrentScene()->AddEntity("wind");
        wind->AddComponent<TransformComponent>();
        MusicComponent *musicComponent = wind->AddComponent<MusicComponent>(this->musWind);
        musicComponent->SetVolume(0.2f);
    }

    if (isAtFunc(14.0f)) {
        context.audioManager->PlaySoundEffect(this->sfxCough, false, 1.0f);
    }

    if (isAtFunc(15.0f)) {
        context.audioManager->PlaySoundEffect(this->sfxGoblin, false, 1.0f);
    }

    if (isAtFunc(17.0f)) {
        context.audioManager->PlaySoundEffect(this->sfxBreathing, false, 1.0f);
    }

    if (isInFunc(18.0f, 23.0f)) {
        img2->SetTexture(this->imgKidnap);

        float t = Ramp(18.0f, 23.0f, this->time);
        img2->SetOpacity(t * t);
    }

    if (isAtFunc(20.0f)) {
        context.audioManager->PlaySoundEffect(this->sfxMjau, false, 2.0f);
    }

    if (isAtFunc(21.0f)) {
        context.audioManager->PlaySoundEffect(this->sfxGoblin, false, 1.0f, 0.7f);
    }

    if (isInFunc(25.0f, 30.0f)) {
        float t = Ramp(25.0f, 30.0f, this->time);
        t = 1.0f - t * t;
        img1->SetOpacity(t);
        img2->SetOpacity(t);
    }

    if (isAtFunc(30.0f)) {
        context.sceneManager->RequestSceneChange("gameplay");
    }
}

void IntroCutsceneComponent::CutsceneDeath(const AppContext &context, float prevTime) {
    auto img1 = this->imgComps[0];
    auto img2 = this->imgComps[1];
    auto txt1 = this->txtComps[0];

    auto isAtFunc = [&](float t) { return t >= prevTime && t < this->time; };
    auto isInFunc = [&](float min, float max) { return this->time >= min && this->time < max; };

    if ((Input::IsKeyPressed(Key::SPACE) || Input::IsGamepadButtonPressed(GamepadButton::SOUTH)) && isInFunc(0.0f, 15.0f)) {
        if (txt1->IsActive())
            this->time = 15.0f;
        else
            txt1->SetActive(true);
    }

    if (isInFunc(0.0f, 5.0f)) {
        float t = Ramp(0.0f, 5.0f, this->time);
        img2->SetOpacity(t * t);
    }

    if (isInFunc(10.0f, 15.0f)) {
        float t = Ramp(10.0f, 15.0f, this->time);
        img2->SetOpacity(1.0f - t * t);
    }

    if (isAtFunc(15.0f)) {
        img1->SetActive(false);
        img2->SetActive(false);
        txt1->SetActive(false);
        Input::SetMouseMode(MouseMode::VISIBLE);
    }
}

void IntroCutsceneComponent::CutsceneWin(const AppContext &context, float prevTime) {
    this->CutsceneDeath(context, prevTime);
}

void IntroCutsceneComponent::OnStart(const AppContext &context) {
    this->imgCuddle = context.assetManager->Get<Texture2D>("assets/images/story/story_1a.png");
    this->imgProne = context.assetManager->Get<Texture2D>("assets/images/story/story_1b_1.png");
    this->imgKidnap = context.assetManager->Get<Texture2D>("assets/images/story/story_1b_2.png");

    this->sfxLaugh = context.assetManager->Get<AudioClip>("assets/audio/female_a/voice_female_a_laugh_12.ogg");
    this->sfxPurr = context.assetManager->Get<AudioClip>("assets/audio/sfx/dog.ogg");
    this->sfxBonk = context.assetManager->Get<AudioClip>("assets/audio/sfx/HeavyAttack/impact_1.ogg");
    this->sfxScream = context.assetManager->Get<AudioClip>("assets/audio/female_a/voice_female_a_hurt_pain_05.ogg");
    this->sfxGoblin = context.assetManager->Get<AudioClip>("assets/audio/sfx/Goblin/202096__spookymodem__goblin-cackle.ogg");
    this->sfxMjau = context.assetManager->Get<AudioClip>("assets/audio/sfx/meow2_2.ogg");
    this->sfxCough = context.assetManager->Get<AudioClip>("assets/audio/female_a/voice_female_a_cough_06.ogg");
    this->sfxBreathing = context.assetManager->Get<AudioClip>("assets/audio/female_a/voice_female_a_breath_deep_out_01.ogg");

    this->musWind = context.assetManager->Get<AudioClip>("assets/audio/sfx/wind-01.ogg");

    Input::SetMouseMode(MouseMode::CAPTURED);
}

void IntroCutsceneComponent::Update(const AppContext &context, float deltaTime) {
    deltaTime = std::min(deltaTime, 0.1f);

    const float prevTime = this->time;
    this->time += deltaTime;

    switch (this->cutscene) {
        case WhichCutsceneAmICurrentlyIn::INTRO:
            this->CutsceneIntro(context, prevTime);
            break;
        case WhichCutsceneAmICurrentlyIn::WIN:
            this->CutsceneWin(context, prevTime);
            break;
        case WhichCutsceneAmICurrentlyIn::DEATH:
            this->CutsceneDeath(context, prevTime);
            break;
    }
}
