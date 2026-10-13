#pragma once

#include "scene_headers.hpp"

namespace Scenes {
    void MainMenu(Scene &outScene, const AppContext &appContext) {
        std::shared_ptr<Font> font = appContext.assetManager->Get<Font>("assets/fonts/YoungSerif-Regular.ttf");

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

        UI::Factory ui(outScene, font);
        SetDefaultStyle(ui.style, *appContext.assetManager);

        Entity *column = UI::Factory::Place(ui.CreateGroup(hud, {440.0f, 566.0f}, "demo_column"), UI::Anchor::TOP_RIGHT, {-180.0f, 300.0f});

        UIButtonComponent *tintButton = UI::Factory::Place(ui.CreateButton(column, "Click me :D", nullptr, "tint_button"), UI::Anchor::TOP, {0.0f, 0.0f});
        UITextComponent *tintLabel = UI::Factory::FindLabel(tintButton->GetOwner());
        std::shared_ptr<int> clickCount = std::make_shared<int>(0);
        tintButton->GetOnClick().Connect([ui, tintButton, tintLabel, clickCount]() {
            ++*clickCount;
            tintLabel->SetText(std::format("Clicked {} time{}", *clickCount, *clickCount == 1 ? "" : "s"));
            ui.FitButtonToText(tintButton);
        });

        UIButtonComponent *swapButton = UI::Factory::Place(ui.CreateButton(column, "Sprite swap", nullptr, "swap_button"), UI::Anchor::TOP, {0.0f, 100.0f});
        UITextComponent *swapLabel = UI::Factory::FindLabel(swapButton->GetOwner());
        swapButton->GetTargetImage()->SetTexture(appContext.assetManager->Get<Texture2D>("assets/images/ui/placeholder_panel.png"));
        swapButton->SetTinted(false);
        swapButton->SetHoveredTexture(appContext.assetManager->Get<Texture2D>("assets/images/ui/placeholder_button_hovered.png"));
        swapButton->SetPressedTexture(appContext.assetManager->Get<Texture2D>("assets/images/ui/placeholder_button_pressed.png"));
        swapButton->SetDisabledTexture(appContext.assetManager->Get<Texture2D>("assets/images/ui/placeholder_button_disabled.png"));
        swapButton->GetOnStateChanged().Connect([ui, swapButton, swapLabel](UI::ButtonState state) {
            swapLabel->SetText(state == UI::ButtonState::DISABLED ? "Sprite swap (disabled)" : "Sprite swap");
            ui.FitButtonToText(swapButton);
        });

        UIButtonComponent *toggleButton = UI::Factory::Place(ui.CreateButton(column, "Disable the one above", nullptr, "toggle_button"), UI::Anchor::TOP, {0.0f, 200.0f});
        UITextComponent *toggleLabel = UI::Factory::FindLabel(toggleButton->GetOwner());
        toggleButton->GetOnClick().Connect([swapButton, toggleLabel]() {
            swapButton->SetInteractable(!swapButton->IsInteractable());
            toggleLabel->SetText(swapButton->IsInteractable() ? "Disable the one above" : "Enable the one above");
        });

        UIButtonComponent *multilineButton = UI::Factory::Place(ui.CreateButton(column, "Buttons can have\nmultiple lines as well", nullptr, "multiline_button"), UI::Anchor::TOP, {-390.0f, 0.0f});

        ui.style.textOutlineThickness = 2.0f;
        ui.style.textOutlineColour = {0.0f, 0.0f, 0.0f, 0.8f};

        UICheckboxComponent *checkbox = ui.CreateCheckbox(column, "Sliders enabled", true, nullptr, "checkbox_row");
        UI::Factory::Place(checkbox, UI::Anchor::TOP_LEFT, {0.0f, 320.0f}, {440.0f, 56.0f});

        ui.style.sliderSize = {440.0f, 28.0f};
        ui.style.sliderHandleSize = {28.0f, 44.0f};

        UITextComponent *volumeLabel = UI::Factory::Place(ui.CreateText(column, "Volume: 50%", 0.0f, "volume_label"), UI::Anchor::TOP_LEFT, {0.0f, 390.0f}, {440.0f, 40.0f});
        volumeLabel->SetAlignment(UI::AlignHorizontal::LEFT, UI::AlignVertical::CENTRE);
        UISliderComponent *volumeSlider = ui.CreateSlider(column, 0.0f, 100.0f, 50.0f, [volumeLabel](float value) {
            volumeLabel->SetText(std::format("Volume: {:.0f}%", std::round(value)));
        }, "volume_slider");
        UI::Factory::Place(volumeSlider, UI::Anchor::TOP_LEFT, {0.0f, 438.0f});

        UITextComponent *difficultyLabel = UI::Factory::Place(ui.CreateText(column, "Increments: 5 / 10", 0.0f, "increment_label"), UI::Anchor::TOP_LEFT, {0.0f, 490.0f}, {440.0f, 40.0f});
        difficultyLabel->SetAlignment(UI::AlignHorizontal::LEFT, UI::AlignVertical::CENTRE);
        UISliderComponent *difficultySlider = ui.CreateSlider(column, 0.0f, 10.0f, 5.0f, [difficultyLabel](float value) {
            difficultyLabel->SetText(std::format("Increment: {:.0f} / 10", std::round(value)));
        }, "increment_slider");
        difficultySlider->SetRange(0.0f, 10.0f, 1.0f);
        UI::Factory::Place(difficultySlider, UI::Anchor::TOP_LEFT, {0.0f, 538.0f});

        UIBarComponent *healthBar = ui.CreateBar(column, 25.0f, 100.0f, "health_bar");
        UI::Factory::Place(healthBar, UI::Anchor::TOP_LEFT, {0.0f, 638.0f});

        ui.style.sliderHandleSize = {44.0f, 28.0f};

        UISliderComponent *verticalSlider = ui.CreateSlider(hud, 0.0f, 1.0f, 0.25f, nullptr, "vertical_slider");
        UI::Factory::Place(verticalSlider, UI::Anchor::TOP_RIGHT, {-110.0f, 300.0f}, {28.0f, 284.0f});
        verticalSlider->SetDirection(UI::FillDirection::BOTTOM_TO_TOP);

        UITextComponent *verticalLabel = ui.CreateText(hud, "25%", 0.0f, "vertical_label");
        UI::Factory::Place(verticalLabel, UI::Anchor::TOP_RIGHT, {-82.0f, 592.0f}, {84.0f, 36.0f});
        verticalLabel->SetAlignment(UI::AlignHorizontal::CENTRE, UI::AlignVertical::CENTRE);
        verticalSlider->GetOnValueChanged().Connect([verticalLabel, healthBar](float value) {
            verticalLabel->SetText(std::format("{:.0f}%", std::round(value * 100.0f)));
            healthBar->SetValue(std::round(value * 100.0f));
        });

        checkbox->GetOnValueChanged().Connect([volumeSlider, difficultySlider, verticalSlider](bool isOn) {
            volumeSlider->SetInteractable(isOn);
            difficultySlider->SetInteractable(isOn);
            verticalSlider->SetInteractable(isOn);
        });
    }
}