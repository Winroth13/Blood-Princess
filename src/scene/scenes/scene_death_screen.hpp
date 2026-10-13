#pragma once

#include "scene_headers.hpp"

namespace Scenes {
    void DeathScreen(Scene &outScene, const AppContext &appContext) {
        std::shared_ptr<Font> font = appContext.assetManager->Get<Font>("assets/fonts/YoungSerif-Regular.ttf");

        Entity *hud = outScene.AddEntity("HUD");
        hud->AddComponent<UIRectComponent>();
        hud->AddComponent<UICanvasComponent>();
        hud->AddComponent<TransformComponent>();
        hud->AddComponent<CameraComponent>();

        UI::Factory ui(outScene, font);
        SetDefaultStyle(ui.style, *appContext.assetManager);

        UI::Factory::Place(ui.CreateText(hud, "YOU ARE DEAD", 100.0f, "title"), UI::Anchor::CENTRE, { 0.0f, -200.0f });

        UIButtonComponent *menu_button = UI::Factory::Place(ui.CreateButton(hud, "Main Menu", nullptr, "menu_button"), UI::Anchor::CENTRE, { 0.0f, 200.0f });
        menu_button->GetOnClick().Connect([appContext]() {
            appContext.sceneManager->RequestSceneChange("main_menu");
            });
    }
}