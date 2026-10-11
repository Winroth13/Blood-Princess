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

        // Test //

        std::shared_ptr<Texture2D> panelSprite = appContext.assetManager->Get<Texture2D>("assets/images/ui/placeholder_panel.png");

        auto addButton = [&](const std::string &name, float y, const std::string &text) {
            Entity *entity = outScene.AddEntity(name);
            entity->SetParent(hud);
            entity->AddComponent<UIRectComponent>()->Place(UI::Anchor::TOP_RIGHT, {-180.0f, y}, {440.0f, 84.0f});

            UIImageComponent *image = entity->AddComponent<UIImageComponent>();
            image->SetTexture(panelSprite);
            image->SetSlicing(14.0f);

            Entity *label = outScene.AddEntity(name + "Label");
            label->SetParent(entity);
            label->AddComponent<UIRectComponent>()->Place(UI::Anchor::STRETCH, {0.0f, 0.0f}, {0.0f, 0.0f});
            UITextComponent *labelText = label->AddComponent<UITextComponent>();
            labelText->SetText(text);
            labelText->SetFontSize(30.0f);
            labelText->SetColour({1.0f, 0.92f, 0.8f, 1.0f});
            labelText->SetAlignment(UI::AlignHorizontal::CENTRE, UI::AlignVertical::CENTRE);

            return std::pair<UIButtonComponent *, UITextComponent *>(entity->AddComponent<UIButtonComponent>(), labelText);
        };

        std::shared_ptr<Texture2D> greySprite = appContext.assetManager->Get<Texture2D>("assets/images/ui/placeholder_button_grey.png");
        auto useTint = [greySprite](UIButtonComponent *button) {
            button->GetTargetImage()->SetTexture(greySprite);
            button->SetColours(
                {0.55f, 0.42f, 0.4f, 1.0f},
                {0.9f, 0.7f, 0.6f, 1.0f},
                {0.8f, 0.2f, 0.22f, 1.0f},
                {0.4f, 0.4f, 0.4f, 0.5f}
            );
        };

        auto [tintButton, tintLabel] = addButton("TintButton", 300.0f, "Tint: click me");
        useTint(tintButton);
        std::shared_ptr<int> clickCount = std::make_shared<int>(0);
        tintButton->GetOnClick().Connect([tintLabel, clickCount]() {
            ++*clickCount;
            tintLabel->SetText("Clicked " + std::to_string(*clickCount) + (*clickCount == 1 ? " time" : " times"));
            LogInfo("TintButton clicked (%d)", *clickCount);
        });

        auto [swapButton, swapLabel] = addButton("SwapButton", 400.0f, "Sprite swap");
        swapButton->SetTinted(false);
        swapButton->SetHoveredTexture(appContext.assetManager->Get<Texture2D>("assets/images/ui/placeholder_button_hovered.png"));
        swapButton->SetPressedTexture(appContext.assetManager->Get<Texture2D>("assets/images/ui/placeholder_button_pressed.png"));
        swapButton->SetDisabledTexture(appContext.assetManager->Get<Texture2D>("assets/images/ui/placeholder_button_disabled.png"));
        swapButton->GetOnStateChanged().Connect([swapLabel](UI::ButtonState state) {
            const bool isDisabled = state == UI::ButtonState::DISABLED;
            swapLabel->SetColour(isDisabled ? XMFLOAT4{0.6f, 0.6f, 0.6f, 0.6f} : XMFLOAT4{1.0f, 0.92f, 0.8f, 1.0f});
            swapLabel->SetText(isDisabled ? "Sprite swap (disabled)" : "Sprite swap");
        });
        swapButton->GetOnClick().Connect([]() { LogInfo("SwapButton clicked"); });

        auto [toggleButton, toggleLabel] = addButton("ToggleButton", 500.0f, "Disable the one above");
        useTint(toggleButton);
        toggleButton->GetOnClick().Connect([swapButton, toggleLabel]() {
            swapButton->SetInteractable(!swapButton->IsInteractable());
            toggleLabel->SetText(swapButton->IsInteractable() ? "Disable the one above" : "Enable the one above");
        });
    }
}