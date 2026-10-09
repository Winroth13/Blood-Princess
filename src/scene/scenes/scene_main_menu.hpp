#pragma once

#include "scene_headers.hpp"

namespace Scenes {
    void MainMenu(Scene &outScene, const AppContext &appContext) {
        Entity *hud = outScene.AddEntity("HUD");
        hud->AddComponent<UIRectComponent>();
        hud->AddComponent<UICanvasComponent>();
        hud->AddComponent<TransformComponent>();
        hud->AddComponent<CameraComponent>();

        Entity *img = outScene.AddEntity("img");
        img->SetParent(hud);
        img->AddComponent<UIRectComponent>()->Place(UI::Anchor::STRETCH, {0.0f, 0.0f}, {0.0f, 0.0f});
        UIImageComponent *imageComp = img->AddComponent<UIImageComponent>();
        imageComp->SetTexture(appContext.assetManager->Get<Texture2D>("assets/images/tower.png"));
        imageComp->SetShouldPreserveAspectRatio(true);
        img->AddComponent<MainMenuComponent>();
    }
}