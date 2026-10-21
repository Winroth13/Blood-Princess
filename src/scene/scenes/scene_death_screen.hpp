#pragma once

#include "scene_headers.hpp"
#include "end_screen.hpp"

namespace Scenes {
    void DeathScreen(Scene &outScene, const AppContext &appContext) {
        std::shared_ptr<Font> font = appContext.assetManager->Get<Font>("assets/fonts/YoungSerif-Regular.ttf");

        Entity *hud = outScene.AddEntity("HUD");
        hud->AddComponent<UIRectComponent>();
        hud->AddComponent<UICanvasComponent>();
        hud->AddComponent<TransformComponent>();
        hud->AddComponent<CameraComponent>();
        hud->AddComponent<MenuScreenComponent>();
        SoundComponent *soundComp = hud->AddComponent<SoundComponent>();
        hud->AddComponent<EnvironmentComponent>();

        soundComp->SetIs3d(false);
        soundComp->SetAutoPlay(true);
        std::shared_ptr<Sound> deathSound = appContext.assetManager->Get<Sound>("assets/sounds/player_death.sound");
        soundComp->SetSound(deathSound);

        UI::Factory ui(outScene, font);
        SetDefaultStyle(ui.style, *appContext.assetManager);

        UI::Factory::Place(ui.CreateText(hud, "YOU DIED", 100.0f, "title"), UI::Anchor::TOP, { 0.0f, 200.0f });

        GameState *gameState = appContext.application->GetGameState();

        DrawScores(hud, gameState, ui, UI::Anchor::CENTRE);

        UIButtonComponent *restartButton = UI::Factory::Place(ui.CreateButton(hud, "RESTART", nullptr, "menu_button"), UI::Anchor::CENTRE, { 0.0f, 250.0f });
        restartButton->GetOnClick().Connect([appContext, gameState]() {
            gameState->clearScores();
            appContext.sceneManager->RequestSceneChange("gameplay");
            });

        UIButtonComponent *menuButton = UI::Factory::Place(ui.CreateButton(hud, "MAIN MENU", nullptr, "menu_button"), UI::Anchor::CENTRE, { 0.0f, 350.0f });
        menuButton->GetOnClick().Connect([appContext]() {
            appContext.sceneManager->RequestSceneChange("main_menu");
            });

        SetDefaultSounds(outScene, appContext, { restartButton, menuButton });
    }
}