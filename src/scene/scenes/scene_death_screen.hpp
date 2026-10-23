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
        IntroCutsceneComponent *cutscene = hud->AddComponent<IntroCutsceneComponent>();

        soundComp->SetIs3d(false);
        soundComp->SetAutoPlay(true);
        std::shared_ptr<Sound> deathSound = appContext.assetManager->Get<Sound>("assets/sounds/player_death.sound");
        soundComp->SetSound(deathSound);

        UI::Factory ui(outScene, font);
        SetDefaultStyle(ui.style, *appContext.assetManager);

        UI::Factory::Place(ui.CreateText(hud, "YOU DIED", 100.0f, "title"), UI::Anchor::TOP, { 0.0f, 200.0f });

        GameState *gameState = appContext.application->GetGameState();

        DrawStats(hud, gameState, ui, UI::Anchor::CENTRE);

        UIButtonComponent *restartButton = UI::Factory::Place(ui.CreateButton(hud, "RESTART", nullptr, "menu_button"), UI::Anchor::CENTRE, { 0.0f, 250.0f });
        restartButton->GetOnClick().Connect([appContext, gameState]() {
            gameState->ClearStats();
            appContext.sceneManager->RequestSceneChange("gameplay");
            });

        UIButtonComponent *menuButton = UI::Factory::Place(ui.CreateButton(hud, "MAIN MENU", nullptr, "menu_button"), UI::Anchor::CENTRE, { 0.0f, 350.0f });
        menuButton->GetOnClick().Connect([appContext]() {
            appContext.sceneManager->RequestSceneChange("main_menu");
            });

        SetDefaultSounds(outScene, appContext, { restartButton, menuButton });

        SetDefaultAmbientSound(outScene, appContext);

        UIImageComponent *blackImage = UI::Factory::Place(ui.CreateImage(hud, nullptr, "black"), UI::Anchor::STRETCH, {0.0f, 0.0f}, {0.0f, 0.0f});
        blackImage->SetColour({0.0f, 0.0f, 0.0f, 1.0f});
        Entity *blackEntity = UI::Factory::GetEntity(blackImage);

        std::shared_ptr<Texture2D> princess = appContext.assetManager->Get<Texture2D>("assets/images/story/story_2.png");
        UIImageComponent *imgPrincess = UI::Factory::Place(ui.CreateImage(blackEntity, princess), UI::Anchor::STRETCH, {0.0f, 0.0f}, {0.0f, 0.0f});
        imgPrincess->SetShouldPreserveAspectRatio(true);
        imgPrincess->SetOpacity(0.0f);

        cutscene->SetWhichCutsceneThatIAmCurrentlyIn(WhichCutsceneAmICurrentlyIn::DEATH);
        cutscene->AddImgComp(blackImage);
        cutscene->AddImgComp(imgPrincess);

        UITextComponent *txt = ui.CreateText(hud, "Click again to skip...");
        UI::Factory::Place(txt, UI::Anchor::BOTTOM_RIGHT, { -20.0f, -20.0f });
        txt->SetActive(false);

        cutscene->SetWhichCutsceneThatIAmCurrentlyIn(WhichCutsceneAmICurrentlyIn::DEATH);
        cutscene->AddImgComp(blackImage);
        cutscene->AddImgComp(imgPrincess);
        cutscene->AddTxtComp(txt);
    }
}