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
            ui.CreateCheckbox(bgEntity, "", app ? app->IsFullscreen() : false, [app](bool isOn) {
                if (app)
                    app->SetFullscreen(isOn);
            }, "fullscreen_checkbox"),
            UI::Anchor::CENTRE,
            { 200.0f, -250.0f }
        );

        UI::Factory::Place(
            ui.CreateText(bgEntity, "Fullscreeen (F11)", 0.0f, "fullscreen_label"),
            UI::Anchor::LEFT,
            {600.0f, -250.0f}
        );

        // Volume
        UI::Factory::Place(
            ui.CreateTitle(bgEntity, "Volume", "volume_title"),
            UI::Anchor::CENTRE,
            {0.0f, -175.0f}
        );

        float masterVolume = app->GetMasterVolume();
        UITextComponent *masterLabel = UI::Factory::Place(
            ui.CreateText(bgEntity, std::format("{:.0f}%", masterVolume * 100), 0.0f, "master_label"),
            UI::Anchor::CENTRE,
            {-60.0f, -100.0f}
        );
        masterLabel->SetAlignHorizontal(UI::AlignHorizontal::RIGHT);
        UISliderComponent *masterDrag = UI::Factory::Place(
            ui.CreateSlider(bgEntity, 0.0f, 2.0f, masterVolume, [app, masterLabel](float value)
                {
                    app->SetMasterVolume(value);
                    masterLabel->SetText(std::format("{:.0f}%", value * 100));
                }, "master_slider"),
            UI::Anchor::CENTRE,
            { 200.0f, -100.f }
        );
        UI::Factory::Place(
            ui.CreateText(bgEntity, "Master", 0.0f, "master_text"),
            UI::Anchor::LEFT,
            { 600.0f, -100.0f }
        );

        float soundVolume = app->GetSoundVolume();
        UITextComponent *soundLabel = UI::Factory::Place(
            ui.CreateText(bgEntity, std::format("{:.0f}%", soundVolume * 100), 0.0f, "sound_label"),
            UI::Anchor::CENTRE,
            { -60.0f, -50.0f }
        );
        soundLabel->SetAlignHorizontal(UI::AlignHorizontal::RIGHT);
        UISliderComponent *soundDrag = UI::Factory::Place(
            ui.CreateSlider(bgEntity, 0.0f, 2.0f, soundVolume, [app, soundLabel](float value)
                {
                    app->SetSoundVolume(value);
                    soundLabel->SetText(std::format("{:.0f}%", value * 100));
                }, "sound_slider"),
            UI::Anchor::CENTRE,
            { 200.0f, -50.f }
        );
        UI::Factory::Place(ui.CreateText(bgEntity, "Sound", 0.0f, "sound_text"), UI::Anchor::LEFT, { 600.0f, -50.0f });

        float musicVolume = app->GetMusicVolume();
        UITextComponent *musicLabel = UI::Factory::Place(
            ui.CreateText(bgEntity, std::format("{:.0f}%", musicVolume * 100), 0.0f, "music_label"),
            UI::Anchor::CENTRE,
            { -60.0f, 0.0f }
        );
        musicLabel->SetAlignHorizontal(UI::AlignHorizontal::RIGHT);
        UISliderComponent *musicDrag = UI::Factory::Place(
            ui.CreateSlider(bgEntity, 0.0f, 2.0f, musicVolume, [app, musicLabel](float value)
                {
                    app->SetMusicVolume(value);
                    musicLabel->SetText(std::format("{:.0f}%", value * 100));
                }, "music_slider"),
            UI::Anchor::CENTRE,
            { 200.0f, 0.f }
        );
        UI::Factory::Place(
            ui.CreateText(bgEntity, "Music", 0.0f, "music_text"),
            UI::Anchor::LEFT,
            { 600.0f, 0.0f }
        );

        //Controls
        UI::Factory::Place(
            ui.CreateTitle(bgEntity, "Controls", "controls_title"),
            UI::Anchor::CENTRE,
            {0.0f, 75.0f}
        );

        float initialSens = app ? app->GetMouseSensitivity() : 0.1f;
        UITextComponent *sensLabel = UI::Factory::Place(
            ui.CreateText(bgEntity, std::format("{:.2f}", initialSens), 0.0f, "sens_label"),
            UI::Anchor::CENTRE,
            { -60.0f, 150.0f }
        );
        UISliderComponent *sensSlider = UI::Factory::Place(
            ui.CreateSlider(bgEntity, 0.01f, 0.50f, initialSens, [app, sensLabel](float value)
                {
                if (app)
                    app->SetMouseSensitivity(value);
                sensLabel->SetText(std::format("{:.2f}", value));
                }, "sens_slider"),
            UI::Anchor::CENTRE,
            { 200.0f, 150.0f }
        );
        sensSlider->SetRange(0.01f, 0.50f, 0.01f);
        UI::Factory::Place(
            ui.CreateText(bgEntity, "Sensitivity", 28.0f, "sens_text"),
            UI::Anchor::LEFT,
            { 600.0f, 150.0f }
        );

        UICheckboxComponent *invertYCheckbox = UI::Factory::Place(
            ui.CreateCheckbox(bgEntity, "", app ? app->GetInvertY() : false, [app](bool isOn) {
                if (app)
                    app->SetInvertY(isOn);
                }, "y_axis_checkbox"),
            UI::Anchor::CENTRE,
            { 200.0f, 200.0f }
        );
        UI::Factory::Place(
            ui.CreateText(bgEntity, "Invert Y-Axis", 0.0f, "y_axis_label"),
            UI::Anchor::LEFT,
            { 600.0f, 200.0f }
        );

        UIButtonComponent *backButton = UI::Factory::Place(ui.CreateButton(bgEntity, "BACK", nullptr, "btn_back"), UI::Anchor::BOTTOM, { 0.0f, -80.0f });
        backButton->GetOnClick().Connect([appContext]()
        {
            appContext.sceneManager->RequestSceneChange("main_menu");
        });

        SetDefaultSounds(outScene, appContext, { backButton });

        SetDefaultAmbientSound(outScene, appContext);
    }
}
