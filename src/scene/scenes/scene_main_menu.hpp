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

        auto addLabel = [&](const std::string &name, Entity *parent, UI::Anchor anchor, XMFLOAT2 position, XMFLOAT2 size, const std::string &text) {
            Entity *entity = outScene.AddEntity(name);
            entity->SetParent(parent);
            entity->AddComponent<UIRectComponent>()->Place(anchor, position, size);
            UITextComponent *labelText = entity->AddComponent<UITextComponent>();
            labelText->SetText(text);
            labelText->SetFontSize(28.0f);
            labelText->SetColour({1.0f, 0.92f, 0.8f, 1.0f});
            labelText->SetOutline(2.0f, {0.0f, 0.0f, 0.0f, 0.8f});
            labelText->SetAlignment(UI::AlignHorizontal::LEFT, UI::AlignVertical::CENTRE);
            return labelText;
        };

        auto addSlider = [&](const std::string &name, UI::Anchor anchor, XMFLOAT2 position, XMFLOAT2 size, XMFLOAT2 handleSize) {
            Entity *entity = outScene.AddEntity(name);
            entity->SetParent(hud);
            entity->AddComponent<UIRectComponent>()->Place(anchor, position, size);

            UIBarComponent *bar = entity->AddComponent<UIBarComponent>();
            bar->SetBackgroundTexture(panelSprite);
            bar->SetBackgroundSliceBorder(14.0f);
            bar->SetBackgroundColour({1.0f, 1.0f, 1.0f, 1.0f});
            bar->SetFillTexture(greySprite);
            bar->SetFillSliceBorder(14.0f);
            bar->SetFillColour({0.75f, 0.08f, 0.12f, 1.0f});
            bar->SetSliceScale(0.5f);
            bar->SetFillPadding(5.0f);
            bar->SetTrailColour({0.0f, 0.0f, 0.0f, 0.0f});

            Entity *handle = outScene.AddEntity(name + "Handle");
            handle->SetParent(entity);
            handle->AddComponent<UIRectComponent>()->Place(UI::Anchor::CENTRE, {0.0f, 0.0f}, handleSize);
            UIImageComponent *handleImage = handle->AddComponent<UIImageComponent>();
            handleImage->SetSlicing(14.0f, 0.5f);

            UISliderComponent *slider = entity->AddComponent<UISliderComponent>();
            slider->SetHandle(handle);

            UIButtonComponent *handleButton = entity->AddComponent<UIButtonComponent>();
            handleButton->SetTargetImage(handleImage);
            handleButton->SetStaysPressedWhenDraggedOff(true);
            useTint(handleButton);

            return slider;
        };

        Entity *checkboxRow = outScene.AddEntity("CheckboxRow");
        checkboxRow->SetParent(hud);
        checkboxRow->AddComponent<UIRectComponent>()->Place(UI::Anchor::TOP_RIGHT, {-180.0f, 620.0f}, {440.0f, 56.0f});
        checkboxRow->AddComponent<UIImageComponent>()->SetColour({0.0f, 0.0f, 0.0f, 0.0f});

        Entity *box = outScene.AddEntity("CheckboxBox");
        box->SetParent(checkboxRow);
        box->AddComponent<UIRectComponent>()->Place(UI::Anchor::LEFT, {0.0f, 0.0f}, {56.0f, 56.0f});
        UIImageComponent *boxImage = box->AddComponent<UIImageComponent>();
        boxImage->SetSlicing(14.0f);

        Entity *checkMark = outScene.AddEntity("CheckMark");
        checkMark->SetParent(box);
        checkMark->AddComponent<UIRectComponent>()->Place(UI::Anchor::STRETCH, {0.0f, 0.0f}, {-14.0f, -14.0f});
        UIImageComponent *checkMarkImage = checkMark->AddComponent<UIImageComponent>();
        checkMarkImage->SetTexture(appContext.assetManager->Get<Texture2D>("assets/images/ui/placeholder_checkmark.png"));
        checkMarkImage->SetColour({1.0f, 0.92f, 0.8f, 1.0f});
        checkMarkImage->SetRaycastTarget(false);

        addLabel("CheckboxLabel", checkboxRow, UI::Anchor::STRETCH, {38.0f, 0.0f}, {-76.0f, 0.0f}, "Sliders enabled");

        UICheckboxComponent *checkbox = checkboxRow->AddComponent<UICheckboxComponent>();
        checkbox->SetCheckMark(checkMark);
        checkbox->SetOn(true);

        UIButtonComponent *checkboxButton = checkboxRow->AddComponent<UIButtonComponent>();
        checkboxButton->SetTargetImage(boxImage);
        useTint(checkboxButton);

        UITextComponent *volumeLabel = addLabel("VolumeLabel", hud, UI::Anchor::TOP_RIGHT, {-180.0f, 690.0f}, {440.0f, 40.0f}, "Volume: 50%");
        UISliderComponent *volumeSlider = addSlider("VolumeSlider", UI::Anchor::TOP_RIGHT, {-180.0f, 738.0f}, {440.0f, 28.0f}, {28.0f, 44.0f});
        volumeSlider->SetRange(0.0f, 100.0f);
        volumeSlider->SetValue(50.0f);
        volumeSlider->GetOnValueChanged().Connect([volumeLabel](float value) {
            volumeLabel->SetText("Volume: " + std::to_string(static_cast<int>(std::round(value))) + "%");
        });

        UITextComponent *difficultyLabel = addLabel("DifficultyLabel", hud, UI::Anchor::TOP_RIGHT, {-180.0f, 790.0f}, {440.0f, 40.0f}, "Difficulty: 5 / 10");
        UISliderComponent *difficultySlider = addSlider("DifficultySlider", UI::Anchor::TOP_RIGHT, {-180.0f, 838.0f}, {440.0f, 28.0f}, {28.0f, 44.0f});
        difficultySlider->SetRange(0.0f, 10.0f, 1.0f);
        difficultySlider->SetValue(5.0f);
        difficultySlider->GetOnValueChanged().Connect([difficultyLabel](float value) {
            difficultyLabel->SetText("Difficulty: " + std::to_string(static_cast<int>(value)) + " / 10");
        });

        UISliderComponent *verticalSlider = addSlider("VerticalSlider", UI::Anchor::TOP_RIGHT, {-110.0f, 300.0f}, {28.0f, 284.0f}, {44.0f, 28.0f});
        verticalSlider->SetDirection(UI::FillDirection::BOTTOM_TO_TOP);
        verticalSlider->SetValue(0.25f);
        UITextComponent *verticalLabel = addLabel("VerticalLabel", hud, UI::Anchor::TOP_RIGHT, {-82.0f, 592.0f}, {84.0f, 36.0f}, "25%");
        verticalLabel->SetAlignment(UI::AlignHorizontal::CENTRE, UI::AlignVertical::CENTRE);
        verticalSlider->GetOnValueChanged().Connect([verticalLabel](float value) {
            verticalLabel->SetText(std::to_string(static_cast<int>(std::round(value * 100.0f))) + "%");
        });

        checkbox->GetOnValueChanged().Connect([volumeSlider, difficultySlider, verticalSlider](bool isOn) {
            volumeSlider->SetInteractable(isOn);
            difficultySlider->SetInteractable(isOn);
            verticalSlider->SetInteractable(isOn);
        });
    }
}