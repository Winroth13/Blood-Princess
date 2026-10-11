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

        Entity *box = outScene.AddEntity("test_interact");
        box->SetParent(hud);
        box->AddComponent<UIRectComponent>()->Place(UI::Anchor::TOP_LEFT, {40.0f, 40.0f}, {360.0f, 80.0f});
        UIImageComponent *boxImage = box->AddComponent<UIImageComponent>();
        UIInteractableComponent *interactable = box->AddComponent<UIInteractableComponent>();

        Entity *icon = outScene.AddEntity("icon");
        icon->SetParent(box);
        icon->AddComponent<UIRectComponent>()->Place(UI::Anchor::LEFT, {16.0f, 0.0f}, {48.0f, 48.0f});
        icon->AddComponent<UIImageComponent>()->SetColour({0.75f, 0.08f, 0.12f, 1.0f});

        Entity *label = outScene.AddEntity("label");
        label->SetParent(box);
        label->AddComponent<UIRectComponent>()->Place(UI::Anchor::STRETCH, {40.0f, 0.0f}, {-80.0f, 0.0f});
        UITextComponent *labelText = label->AddComponent<UITextComponent>();
        labelText->SetText("Click me");
        labelText->SetFontSize(28.0f);
        labelText->SetColour({1.0f, 1.0f, 1.0f, 1.0f});
        labelText->SetAlignment(UI::AlignHorizontal::LEFT, UI::AlignVertical::CENTRE);

        const XMFLOAT4 normalColour = {0.12f, 0.08f, 0.1f, 0.85f};
        const XMFLOAT4 hoveredColour = {0.3f, 0.2f, 0.24f, 0.95f};
        const XMFLOAT4 pressedColour = {0.06f, 0.04f, 0.05f, 1.0f};
        boxImage->SetColour(normalColour);

        interactable->GetOnPointerEnter().Connect([boxImage, hoveredColour]() { boxImage->SetColour(hoveredColour); });
        interactable->GetOnPointerExit().Connect([boxImage, normalColour]() { boxImage->SetColour(normalColour); });
        interactable->GetOnPointerDown().Connect([boxImage, pressedColour](XMFLOAT2 position) { boxImage->SetColour(pressedColour); });
        interactable->GetOnPointerUp().Connect([boxImage, interactable, normalColour, hoveredColour](XMFLOAT2 position) {
            boxImage->SetColour(interactable->IsHovered() ? hoveredColour : normalColour);
        });

        std::shared_ptr<int> clickCount = std::make_shared<int>(0);
        interactable->GetOnClick().Connect([labelText, clickCount]() {
            ++*clickCount;
            labelText->SetText("Clicked " + std::to_string(*clickCount) + (*clickCount == 1 ? " time" : " times"));
            LogInfo("InteractDemo clicked (%d)", *clickCount);
        });
    }
}