#pragma once

#include "scene_headers.hpp"

struct listDesc {
    std::string title;
    std::vector<std::string> listItems;
    float itemSize = 35.0f;
};

inline Entity *CreateCredits(Entity *parent, UI::Factory &ui)
{
    float listPadding = 100.0f;

    Entity *credits = ui.CreateGroup(parent, { 0.0f, 0.0f }, "credits");
    UI::Factory::Place(credits, UI::Anchor::TOP, { 0.0f, 0.0f });

    std::vector<listDesc> creditsLists =
    {
        {
            "Programming",
            {
                "Emil Winroth",
                "Gustav Källström",
                "Casper Turesson",
                "Johan Wisell",
                "Johannes Ledendal",
                "Mohammad Othman"
            }
        },
        {
            "3D-Modelling",
            {
                "Gustav Källström",
                "Johannes Ledendal"
            }
        },
        {
            "Illustrations",
            {
                "Casper Turesson",
                "Britt-Marie (Cat)"
            }
        },
        {
            "Voice Acting",
            {
                "Britt-Marie (Cat)"
            }
        },
        {
            "Assets",
            {
                "'Skeleton Rigged Model' by RosyJoan",
                "(https://rosyjoan.itch.io/skeleton-riggedmodel)"
            },
            25.0f
        }
    };

    float height = 0;

    for (listDesc list : creditsLists)
    {
        Entity *listEntity = CreateTitledList(credits, ui, UI::Anchor::TOP, { 0.0f, height }, list.title, list.listItems, list.itemSize);
        height += UI::Factory::GetRect(listEntity)->GetSizeDelta().y + listPadding;
    }

    height -= listPadding;

    UI::Factory::GetRect(credits)->SetSizeDelta({ 500.0f, height });

    return credits;
}

namespace Scenes {
    void Credits(Scene &outScene, const AppContext &appContext) {
        std::shared_ptr<Font> font = appContext.assetManager->Get<Font>("assets/fonts/YoungSerif-Regular.ttf");

        UI::Factory ui(outScene, font);
        SetDefaultStyle(ui.style, *appContext.assetManager);

        Entity *canvas = ui.CreateCanvas();
        canvas->AddComponent<TransformComponent>();
        canvas->AddComponent<CameraComponent>();
        canvas->AddComponent<MenuScreenComponent>();
        canvas->AddComponent<EnvironmentComponent>();

        UI::Factory::Place(ui.CreateImage(canvas, nullptr, "black"), UI::Anchor::STRETCH, { 0.0f, 0.0f }, { 0.0f, 0.0f })->SetColour({ 0.0f, 0.0f, 0.0f, 1.0f });

        std::shared_ptr<Texture2D> bgTexture = appContext.assetManager->Get<Texture2D>("assets/images/johannes_tower.png");
        UIImageComponent *bgImage = UI::Factory::Place(ui.CreateImage(canvas, bgTexture, "background"), UI::Anchor::STRETCH, { 0.0f, 0.0f }, { 0.0f, 0.0f });
        Entity *bgEntity = UI::Factory::GetEntity(bgImage);
        UIRectComponent *bgRect = UI::Factory::GetRect(bgEntity);
        bgRect->SetAspectRatio(bgImage->GetAspectRatio());
        bgRect->SetReferenceHeight(bgImage->GetSpriteSize().y);
        bgImage->SetOpacity(0.0f);

        Entity *credits = CreateCredits(bgEntity, ui);
        credits->AddComponent<CreditsComponent>()->SetHeights(220.0f, 820.0f);
        
        // Hide credits outside of background when aspect ratio is not 16:9
        UI::Factory::Place(ui.CreateImage(bgEntity, nullptr, "black_top"), UI::Anchor::STRETCHED_HORIZONTAL_TOP, { 0.0f, -500.0f }, { 0.0f, 500.0f })->SetColour({ 0.0f, 0.0f, 0.0f, 1.0f });
        UI::Factory::Place(ui.CreateImage(bgEntity, nullptr, "black_bottom"), UI::Anchor::STRETCHED_HORIZONTAL_BOTTOM, { 0.0f, 500.0f }, { 0.0f, 500.0f })->SetColour({ 0.0f, 0.0f, 0.0f, 1.0f });

        // Fade in/out credits
        std::shared_ptr<Texture2D> topFadeTexture = appContext.assetManager->Get<Texture2D>("assets/images/fade_top.png");
        UI::Factory::Place(ui.CreateImage(bgEntity, topFadeTexture, "top_bar"), UI::Anchor::STRETCHED_HORIZONTAL_TOP, { 0.0f, 0.0f }, { 0.0f, 220.0f + 32.0f });

        std::shared_ptr<Texture2D> bottomFadeTexture = appContext.assetManager->Get<Texture2D>("assets/images/fade_bot.png");
        UI::Factory::Place(ui.CreateImage(bgEntity, bottomFadeTexture, "bottom_bar"), UI::Anchor::STRETCHED_HORIZONTAL_BOTTOM, { 0.0f, 0.0f }, { 0.0f, 180.0f + 32.0f });

        UITextComponent *titleText = ui.CreateText(bgEntity, "CREDITS", 100.0f, "title");
        UI::Factory::Place(titleText, UI::Anchor::TOP, { 0.0f, 80.0f });
        titleText->SetAlignHorizontal(UI::AlignHorizontal::CENTRE);
        titleText->SetColour(UI::RGBA{ 25, 17, 6, 255 });
        titleText->SetLineSpacing(0.8f);
        titleText->SetOutline(7.0f, UI::RGBA{ 103, 44, 32, 255 });
        titleText->SetShadow({ 8.0f, 8.0f }, UI::RGBA{ 0, 0, 0, 136 });

        ui.style.fontSizeRegular = 28.0f;

        UIButtonComponent *backButton = UI::Factory::Place(ui.CreateButton(bgEntity, "BACK", nullptr, "btn_back"), UI::Anchor::BOTTOM, { 0.0f, -80.0f });
        backButton->GetOnClick().Connect([appContext]() {
            appContext.sceneManager->RequestSceneChange("main_menu");
            });

        SetDefaultSounds(outScene, appContext, { backButton });

        SetDefaultAmbientSound(outScene, appContext);
    }
}