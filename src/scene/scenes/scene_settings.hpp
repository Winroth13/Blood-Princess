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
        canvas->AddComponent<MenuScreenComponent>();
        canvas->AddComponent<EnvironmentComponent>();

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
        UICheckboxComponent *fullscreenCheckbox = UI::Factory::Place(
            ui.CreateCheckbox(bgEntity, "Fullscreen (F11)", app ? app->IsFullscreen() : false, [app](bool isOn) {
                if (app)
                    app->SetFullscreen(isOn);
            }),
            UI::Anchor::TOP,
            {0.0f, 220.0f}
        );

        float initialSens = app ? app->GetMouseSensitivity() : 0.1f;
        UITextComponent *sensLabel = UI::Factory::Place(
            ui.CreateText(bgEntity, std::format("Sensitivity: {:.2f}", initialSens), 28.0f, "sens_label"),
            UI::Anchor::TOP,
            {0.0f, 290.0f}
        );
        sensLabel->SetAlignHorizontal(UI::AlignHorizontal::CENTRE);

        UISliderComponent *sensSlider = UI::Factory::Place(
            ui.CreateSlider(bgEntity, 0.01f, 0.50f, initialSens, [app, sensLabel](float value) {
                if (app)
                    app->SetMouseSensitivity(value);
                sensLabel->SetText(std::format("Sensitivity: {:.2f}", value));
            }, "slider_sens"),
            UI::Anchor::TOP,
            {0.0f, 340.0f}
        );
        sensSlider->SetRange(0.01f, 0.50f, 0.01f);

        UICheckboxComponent *invertYCheckbox = UI::Factory::Place(
            ui.CreateCheckbox(bgEntity, "Invert Y-Axis", app ? app->GetInvertY() : false, [app](bool isOn) {
                if (app)
                    app->SetInvertY(isOn);
            }),
            UI::Anchor::TOP,
            {0.0f, 410.0f}
        );

        UIButtonComponent *backButton = UI::Factory::Place(ui.CreateButton(bgEntity, "BACK", nullptr, "btn_back"), UI::Anchor::BOTTOM, { 0.0f, -80.0f });
        backButton->GetOnClick().Connect([appContext]() {
            appContext.sceneManager->RequestSceneChange("main_menu");
        });

        SetDefaultSounds(outScene, appContext, { backButton });

        SetDefaultAmbientSound(outScene, appContext);
    }
}
