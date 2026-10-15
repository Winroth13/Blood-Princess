#pragma once

#include "scene_headers.hpp"

namespace Scenes {
    void Settings(Scene &outScene, const AppContext &appContext) {
        std::shared_ptr<Font> font = appContext.assetManager->Get<Font>("assets/fonts/YoungSerif-Regular.ttf");

        UI::Factory ui(outScene, font);
        SetDefaultStyle(ui.style, *appContext.assetManager);

        Entity *canvas = ui.CreateCanvas();
        canvas->AddComponent<TransformComponent>();
        canvas->AddComponent<CameraComponent>();

        UIImageComponent *blackImage = UI::Factory::Place(ui.CreateImage(canvas, nullptr, "black"), UI::Anchor::STRETCH, {0.0f, 0.0f}, {0.0f, 0.0f});
        blackImage->SetColour({0.0f, 0.0f, 0.0f, 1.0f});

        std::shared_ptr<Texture2D> bgTexture = appContext.assetManager->Get<Texture2D>("assets/images/johannes_tower.png");
        UIImageComponent *bgImage = UI::Factory::Place(ui.CreateImage(canvas, bgTexture, "background"), UI::Anchor::STRETCH, {0.0f, 0.0f}, {0.0f, 0.0f});
        Entity *bgEntity = UI::Factory::GetEntity(bgImage);
        UIRectComponent *bgRect = UI::Factory::GetRect(bgEntity);
        bgRect->SetAspectRatio(bgImage->GetAspectRatio());
        bgRect->SetReferenceHeight(bgImage->GetSpriteSize().y);
        bgImage->SetOpacity(0.0f);

        UITextComponent *titleText = ui.CreateText(bgEntity, "SETTINGS", 100.0f, "title");
        UI::Factory::Place(titleText, UI::Anchor::TOP, {0.0f, 80.0f});
        titleText->SetAlignHorizontal(UI::AlignHorizontal::CENTRE);
        titleText->SetColour(UI::RGBA{25, 17, 6, 255});
        titleText->SetLineSpacing(0.8f);
        titleText->SetOutline(7.0f, UI::RGBA{103, 44, 32, 255});
        titleText->SetShadow({8.0f, 8.0f}, UI::RGBA{0, 0, 0, 136});

        ui.style.fontSizeRegular = 28.0f;

        Application *app = appContext.application;
        UICheckboxComponent *checkbox = ui.CreateCheckbox(bgEntity, "Fullscreeen? (F11)", false, [app](bool isOn) {
            app->SetFullscreen(isOn);
        });

        UIButtonComponent *backButton = UI::Factory::Place(ui.CreateButton(bgEntity, "BACK", nullptr, "btn_back"), UI::Anchor::BOTTOM, { 0.0f, -80.0f });
        backButton->GetOnClick().Connect([appContext]() {
            appContext.sceneManager->RequestSceneChange("main_menu");
        });

        SetDefaultSounds(outScene, appContext, { backButton });
    }
}