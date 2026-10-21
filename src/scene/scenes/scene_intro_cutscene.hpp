#pragma once

#include "scene_headers.hpp"

namespace Scenes {
    void IntroCutscene(Scene &outScene, const AppContext &appContext) {
        UI::Factory ui(outScene, *appContext.assetManager);
        SetDefaultStyle(ui.style, *appContext.assetManager);

        Entity *canvas = ui.CreateCanvas();
        canvas->AddComponent<TransformComponent>();
        canvas->AddComponent<CameraComponent>();
        canvas->AddComponent<EnvironmentComponent>();

        UIImageComponent *blackImage = UI::Factory::Place(ui.CreateImage(canvas, nullptr, "black"), UI::Anchor::STRETCH, {0.0f, 0.0f}, {0.0f, 0.0f});
        blackImage->SetColour({0.0f, 0.0f, 0.0f, 1.0f});

        Entity *cutsceneGroup = ui.CreateGroup(canvas, {0.0f, 0.0f}, "cutscene_group");
        UI::Factory::GetRect(cutsceneGroup)->SetAnchorPreset(UI::Anchor::STRETCH);
        cutsceneGroup->AddComponent<IntroCutsceneComponent>();

        UIImageComponent *imgCutscene1 = ui.CreateImage(cutsceneGroup, nullptr, "image_1");
        UI::Factory::Place(imgCutscene1, UI::Anchor::STRETCH, {0.0f, 0.0f}, {0.0f, 0.0f});
        imgCutscene1->SetShouldPreserveAspectRatio(true);
        imgCutscene1->SetOpacity(0.0f);

        UIImageComponent *imgCutscene2 = ui.CreateImage(cutsceneGroup, nullptr, "image_2");
        UI::Factory::Place(imgCutscene2, UI::Anchor::STRETCH, {0.0f, 0.0f}, {0.0f, 0.0f});
        imgCutscene2->SetShouldPreserveAspectRatio(true);
        imgCutscene2->SetOpacity(0.0f);
    }
}