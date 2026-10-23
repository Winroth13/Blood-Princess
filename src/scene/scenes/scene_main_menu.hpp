#pragma once

#include "scene_headers.hpp"

namespace Scenes {
    void MainMenu(Scene &outScene, const AppContext &appContext) {
        std::shared_ptr<Font> font = appContext.assetManager->Get<Font>("assets/fonts/YoungSerif-Regular.ttf");

        UI::Factory ui(outScene, font);
        SetDefaultStyle(ui.style, *appContext.assetManager);

        Entity *canvas = ui.CreateCanvas();
        canvas->AddComponent<TransformComponent>();
        canvas->AddComponent<CameraComponent>();
        canvas->AddComponent<MainMenuComponent>();
        canvas->AddComponent<EnvironmentComponent>();

        UIImageComponent *blackImage = UI::Factory::Place(ui.CreateImage(canvas, nullptr, "black"), UI::Anchor::STRETCH, {0.0f, 0.0f}, {0.0f, 0.0f});
        blackImage->SetColour({0.0f, 0.0f, 0.0f, 1.0f});

        std::shared_ptr<Texture2D> bgTexture = appContext.assetManager->Get<Texture2D>("assets/images/johannes_tower.png");
        UIImageComponent *bgImage = UI::Factory::Place(ui.CreateImage(canvas, bgTexture, "background"), UI::Anchor::STRETCH, {0.0f, 0.0f}, {0.0f, 0.0f});
        Entity *bgEntity = UI::Factory::GetEntity(bgImage);
        UIRectComponent *bgRect = UI::Factory::GetRect(bgEntity);
        bgRect->SetAspectRatio(bgImage->GetAspectRatio());
        bgRect->SetReferenceHeight(bgImage->GetSpriteSize().y);

        UIImageComponent *loadingImage = UI::Factory::Place(ui.CreateImage(canvas, nullptr, "loading"), UI::Anchor::STRETCH, {0.0f, 0.0f}, {0.0f, 0.0f});
        loadingImage->SetColour({0.0f, 0.0f, 0.0f, 1.0f});
        Entity *loadingEntity = UI::Factory::GetEntity(loadingImage);
        loadingEntity->SetLocallyActive(false);
        // UITextComponent *loadingText = UI::Factory::Place(ui.CreateTitle(loadingEntity, "Loading..."), UI::Anchor::CENTRE, {0.0f, 0.0f}, {0.0f, 0.0f});

        std::shared_ptr<Texture2D> princessTexture = appContext.assetManager->Get<Texture2D>("assets/images/princess_trans.png");
        UIImageComponent *princessImage = ui.CreateImage(bgEntity, princessTexture, "princess");
        UI::Factory::Place(princessImage, UI::Anchor::TOP_LEFT, {860.0f, 650.0f}, {200.0f, 200.0f});
        princessImage->SetShouldPreserveAspectRatio(true);
        princessImage->SetColour(UI::RGBA{165, 116, 99, 255});

        UITextComponent *titleText = ui.CreateText(bgEntity, "BLOOD\nPRINCESS", 100.0f, "title");
        UI::Factory::Place(titleText, UI::Anchor::TOP_LEFT, {35.0f, 80.0f});
        titleText->SetAlignHorizontal(UI::AlignHorizontal::CENTRE);
        titleText->SetColour(UI::RGBA{25, 17, 6, 255});
        titleText->SetLineSpacing(0.8f);
        titleText->SetOutline(7.0f, UI::RGBA{ 186, 80, 52, 255});
        titleText->SetShadow({8.0f, 8.0f}, UI::RGBA{0, 0, 0, 136});

        ui.style.fontSizeRegular = 42.0f;
        UIButtonComponent *btnStart = ui.CreateButton(bgEntity, "START", nullptr, "btn_start");
        UI::Factory::Place(btnStart, UI::Anchor::TOP_LEFT, {50.0f, 380.0f});

        ui.style.fontSizeRegular = 28.0f;
        UIButtonComponent *btnSettings = ui.CreateButton(bgEntity, "SETTINGS", nullptr, "btn_settings");
        UI::Factory::Place(btnSettings, UI::Anchor::TOP_LEFT, {50.0f, 500.0f});

        UIButtonComponent *btnCredits = ui.CreateButton(bgEntity, "CREDITS", nullptr, "btn_credits");
        UI::Factory::Place(btnCredits, UI::Anchor::TOP_LEFT, {50.0f, 600.0f});

        UIButtonComponent *btnQuit = ui.CreateButton(bgEntity, "QUIT", nullptr, "btn_quit");
        UI::Factory::Place(btnQuit, UI::Anchor::TOP_LEFT, {50.0f, 700.0f});

        SceneManager *sceneManager = appContext.sceneManager;
        GameState *gameState = appContext.application->GetGameState();
        btnStart->GetOnClick().Connect([loadingEntity, sceneManager, gameState]() {  
            loadingEntity->SetLocallyActive(true);
            gameState->ClearStats();
            sceneManager->RequestSceneChange("intro_cutscene");
        });

        btnSettings->GetOnClick().Connect([sceneManager]() {  
            sceneManager->RequestSceneChange("settings");
        });

        btnCredits->GetOnClick().Connect([sceneManager]()
        {
            sceneManager->RequestSceneChange("credits");
        });

        Application *app = appContext.application;
        btnQuit->GetOnClick().Connect([app]() {
            app->Stop();
        });

        SetDefaultSounds(outScene, appContext, { btnStart, btnSettings, btnCredits, btnQuit });

        SetDefaultAmbientSound(outScene, appContext);

        //std::string controlsText = "Controls\nMovement: WASD\nLight Attack: LMB\nHeavy Attack: RMB\nParry: F\nDash: Shift/Space";
        //UITextComponent *controls = ui.CreateText(bgEntity, controlsText, 30.0f, "controls");
        //UI::Factory::Place(controls, UI::Anchor::TOP_RIGHT, { -100.0f, 380.0f });
    }
}