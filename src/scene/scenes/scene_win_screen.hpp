#pragma once

#include "scene_headers.hpp"

namespace Scenes {
    void WinScreen(Scene &outScene, const AppContext &appContext) {
        std::shared_ptr<Font> font = appContext.assetManager->Get<Font>("assets/fonts/YoungSerif-Regular.ttf");

        Entity *hud = outScene.AddEntity("HUD");
        hud->AddComponent<UIRectComponent>();
        hud->AddComponent<UICanvasComponent>();
        hud->AddComponent<TransformComponent>();
        hud->AddComponent<CameraComponent>();
        hud->AddComponent<EndScreenComponent>();

        UI::Factory ui(outScene, font);
        SetDefaultStyle(ui.style, *appContext.assetManager);

        UI::Factory::Place(ui.CreateText(hud, "YOU WON", 100.0f, "title"), UI::Anchor::CENTRE, { 0.0f, -200.0f });

        UIButtonComponent *menuButton = UI::Factory::Place(ui.CreateButton(hud, "Main Menu", nullptr, "menu_button"), UI::Anchor::CENTRE, { 0.0f, 200.0f });
        menuButton->GetOnClick().Connect([appContext]() {
            appContext.sceneManager->RequestSceneChange("main_menu");
            });

        // Button sounds
        std::shared_ptr<AudioClip> hoveredClip = appContext.assetManager->Get<AudioClip>("assets/audio/sfx/LightAttack/sword_swing_1.ogg");
        std::shared_ptr<AudioClip> pressedClip = appContext.assetManager->Get<AudioClip>("assets/audio/sfx/LightAttack/sword_parry_1.ogg");

        AudioManager *audioManager = appContext.audioManager;
        auto btnSoundFunc = [audioManager, pressedClip, hoveredClip](UI::ButtonState state) {
            if (state == UI::ButtonState::HOVERED)
                audioManager->PlaySoundEffect(hoveredClip, false);
            else if (state == UI::ButtonState::PRESSED)
                audioManager->PlaySoundEffect(pressedClip, false, 0.4f);
            };

        menuButton->GetOnStateChanged().Connect(btnSoundFunc);

        // Ambient sound
        Entity *wind = outScene.AddEntity("wind");
        wind->AddComponent<TransformComponent>();
        MusicComponent *musicComponent = wind->AddComponent<MusicComponent>(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/wind-01.ogg"));
        musicComponent->SetVolume(0.2f);
    }
}