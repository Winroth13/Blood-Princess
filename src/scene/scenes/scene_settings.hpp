#pragma once

#include "scene_headers.hpp"
#include "input/action.hpp"

namespace Scenes {
    class SettingsRebindComponent : public Component {
        RegisterComponent(SettingsRebindComponent);

    public:
        enum class Column {
            INPUT1,
            INPUT2,
            CONTROLLER
        };

        struct RowElements {
            std::string actionName;
            UIButtonComponent* btnInput1 = nullptr;
            UITextComponent* labelInput1 = nullptr;
            UIButtonComponent* btnInput2 = nullptr;
            UITextComponent* labelInput2 = nullptr;
            UIButtonComponent* btnController = nullptr;
            UITextComponent* labelController = nullptr;
        };

    private:
        std::vector<RowElements> rows;
        bool isRebinding = false;
        std::string rebindingAction;
        Column rebindingCol = Column::INPUT1;
        UIButtonComponent* activeBtn = nullptr;
        UITextComponent* activeLabel = nullptr;
        std::string previousLabelText;
        int ignoreMouseFrames = 0;
        DirectX::XMFLOAT4 originalNormalColour = {0.55f, 0.42f, 0.4f, 1.0f};
        DirectX::XMFLOAT4 originalHoveredColour = {0.9f, 0.7f, 0.6f, 1.0f};

    public:
        SettingsRebindComponent(Entity *owner, bool isActive) : Component(owner, isActive) {
            UsesUpdate();
        }

        void AddRow(const RowElements& row) {
            rows.push_back(row);
        }

        void RefreshAllLabels() {
            for (auto& row : rows) {
                ActionBinding* binding = Action::GetBinding(row.actionName);
                if (!binding) continue;
                if (row.labelInput1) row.labelInput1->SetText(Action::GetSlotDisplayName(binding->input1));
                if (row.labelInput2) row.labelInput2->SetText(Action::GetSlotDisplayName(binding->input2));
                if (row.labelController) row.labelController->SetText(Action::GetGamepadButtonDisplayName(binding->controllerInput));
            }
        }

        void StartRebinding(const std::string& actionName, Column col, UIButtonComponent* btn, UITextComponent* label) {
            if (isRebinding) {
                // Clicking the active rebinding button again with Left Mouse binds Left Mouse!
                if (activeBtn == btn && rebindingCol != Column::CONTROLLER) {
                    ActionSlot slot = ActionSlot::FromMouse(MouseButton::LEFT);
                    if (rebindingCol == Column::INPUT1) {
                        Action::SetInput1(rebindingAction, slot);
                    } else {
                        Action::SetInput2(rebindingAction, slot);
                    }
                    FinishRebinding(Action::GetSlotDisplayName(slot));
                    return;
                }
                CancelRebinding();
            }

            isRebinding = true;
            rebindingAction = actionName;
            rebindingCol = col;
            activeBtn = btn;
            activeLabel = label;
            previousLabelText = label ? label->GetText() : "";
            ignoreMouseFrames = 2;

            if (btn) {
                originalNormalColour = btn->GetNormalColour();
                originalHoveredColour = btn->GetHoveredColour();
                const DirectX::XMFLOAT4 highlightColour = {0.85f, 0.22f, 0.2f, 1.0f};
                btn->SetNormalColour(highlightColour);
                btn->SetHoveredColour(highlightColour);
            }

            if (label) {
                label->SetText("[PRESS]");
            }
        }

        void CancelRebinding() {
            if (!isRebinding) return;
            if (activeBtn) {
                activeBtn->SetNormalColour(originalNormalColour);
                activeBtn->SetHoveredColour(originalHoveredColour);
            }
            if (activeLabel) {
                activeLabel->SetText(previousLabelText);
            }
            isRebinding = false;
            activeBtn = nullptr;
            activeLabel = nullptr;
        }

        void FinishRebinding(const std::string& newText) {
            if (activeBtn) {
                activeBtn->SetNormalColour(originalNormalColour);
                activeBtn->SetHoveredColour(originalHoveredColour);
            }
            if (activeLabel) {
                activeLabel->SetText(newText);
            }
            Action::SaveBindings("bindings.cfg");
            isRebinding = false;
            activeBtn = nullptr;
            activeLabel = nullptr;
        }

        void Update(const AppContext& context, float deltaTime) override {
            if (!isRebinding) return;

            if (ignoreMouseFrames > 0) {
                ignoreMouseFrames--;
            }

            // Cancel with Escape
            if (Input::IsKeyPressed(Key::ESCAPE)) {
                CancelRebinding();
                return;
            }

            // Clear with Delete or Backspace
            if (Input::IsKeyPressed(Key::DELETE_KEY) || Input::IsKeyPressed(Key::BACKSPACE)) {
                if (rebindingCol == Column::INPUT1) {
                    Action::SetInput1(rebindingAction, ActionSlot{});
                } else if (rebindingCol == Column::INPUT2) {
                    Action::SetInput2(rebindingAction, ActionSlot{});
                } else if (rebindingCol == Column::CONTROLLER) {
                    Action::SetControllerInput(rebindingAction, GamepadButton::UNKNOWN);
                }
                FinishRebinding("---");
                return;
            }

            if (rebindingCol == Column::INPUT1 || rebindingCol == Column::INPUT2) {
                // Check keyboard key
                Key pressedKey = Input::GetAnyKeyPressed();
                if (pressedKey != Key::UNKNOWN && pressedKey != Key::ESCAPE && pressedKey != Key::DELETE_KEY && pressedKey != Key::BACKSPACE) {
                    ActionSlot slot = ActionSlot::FromKey(pressedKey);
                    if (rebindingCol == Column::INPUT1) {
                        Action::SetInput1(rebindingAction, slot);
                    } else {
                        Action::SetInput2(rebindingAction, slot);
                    }
                    FinishRebinding(Action::GetSlotDisplayName(slot));
                    return;
                }

                // Check mouse buttons (Right, Middle, X1, X2 can be bound by clicking)
                if (ignoreMouseFrames == 0) {
                    if (Input::IsMouseButtonPressed(MouseButton::RIGHT)) {
                        ActionSlot slot = ActionSlot::FromMouse(MouseButton::RIGHT);
                        if (rebindingCol == Column::INPUT1) Action::SetInput1(rebindingAction, slot);
                        else Action::SetInput2(rebindingAction, slot);
                        FinishRebinding(Action::GetSlotDisplayName(slot));
                        return;
                    }
                    if (Input::IsMouseButtonPressed(MouseButton::MIDDLE)) {
                        ActionSlot slot = ActionSlot::FromMouse(MouseButton::MIDDLE);
                        if (rebindingCol == Column::INPUT1) Action::SetInput1(rebindingAction, slot);
                        else Action::SetInput2(rebindingAction, slot);
                        FinishRebinding(Action::GetSlotDisplayName(slot));
                        return;
                    }
                    if (Input::IsMouseButtonPressed(MouseButton::X1)) {
                        ActionSlot slot = ActionSlot::FromMouse(MouseButton::X1);
                        if (rebindingCol == Column::INPUT1) Action::SetInput1(rebindingAction, slot);
                        else Action::SetInput2(rebindingAction, slot);
                        FinishRebinding(Action::GetSlotDisplayName(slot));
                        return;
                    }
                    if (Input::IsMouseButtonPressed(MouseButton::X2)) {
                        ActionSlot slot = ActionSlot::FromMouse(MouseButton::X2);
                        if (rebindingCol == Column::INPUT1) Action::SetInput1(rebindingAction, slot);
                        else Action::SetInput2(rebindingAction, slot);
                        FinishRebinding(Action::GetSlotDisplayName(slot));
                        return;
                    }
                }
            }
            else if (rebindingCol == Column::CONTROLLER) {
                GamepadButton pressedGamepad = Input::GetAnyGamepadButtonPressed();
                if (pressedGamepad != GamepadButton::UNKNOWN) {
                    Action::SetControllerInput(rebindingAction, pressedGamepad);
                    FinishRebinding(Action::GetGamepadButtonDisplayName(pressedGamepad));
                    return;
                }
            }
        }
    };

    std::vector<UIButtonComponent *> CreateGeneralSettingsTab(UI::Factory ui, AppContext appContext, Entity *generalGroup, float topY)
    {
        std::vector<UIButtonComponent *> generalBtns{};

        ui.style.fontSizeRegular = 28.0f;

        Application *app = appContext.application;
        UICheckboxComponent *fullscreenCheckbox = UI::Factory::Place(
            ui.CreateCheckbox(generalGroup, "", app ? app->IsFullscreen() : false, [app](bool isOn) {
                if (app)
                    app->SetFullscreen(isOn);
                }, "fullscreen_checkbox"),
            UI::Anchor::TOP,
            { 200.0f, topY + 6.0f }
        );

        UI::Factory::Place(
            ui.CreateText(generalGroup, "Fullscreeen (F11)", 0.0f, "fullscreen_label"),
            UI::Anchor::TOP_LEFT,
            { 600.0f, topY + 10.0f }
        )->SetAlignVertical(UI::AlignVertical::CENTRE);

        UICheckboxComponent *skipTutorialCheckbox = UI::Factory::Place(
            ui.CreateCheckbox(generalGroup, "", app ? app->IsFullscreen() : false, [app](bool isOn) {
                if (app)
                    app->SetFullscreen(isOn);
                }, "skip_tutorial_checkbox"),
            UI::Anchor::TOP,
            { 200.0f, topY + 76.0f }
        );

        UI::Factory::Place(
            ui.CreateText(generalGroup, "Skip Tutorial", 0.0f, "skip_tutorial_label"),
            UI::Anchor::TOP_LEFT,
            { 600.0f, topY + 80.0f }
        )->SetAlignVertical(UI::AlignVertical::CENTRE);

        // Volume
        UI::Factory::Place(
            ui.CreateTitle(generalGroup, "Volume", "volume_title"),
            UI::Anchor::TOP,
            { 0.0f, topY + 125.0f }
        )->SetAlignVertical(UI::AlignVertical::CENTRE);

        float masterVolume = app->GetMasterVolume();
        UITextComponent *masterLabel = UI::Factory::Place(
            ui.CreateText(generalGroup, std::format("{:.0f}%", masterVolume * 100), 0.0f, "master_label"),
            UI::Anchor::TOP,
            { -60.0f, topY + 200.0f }
        );
        masterLabel->SetAlignVertical(UI::AlignVertical::CENTRE);
        masterLabel->SetAlignHorizontal(UI::AlignHorizontal::RIGHT);
        UISliderComponent *masterDrag = UI::Factory::Place(
            ui.CreateSlider(generalGroup, 0.0f, 2.0f, masterVolume, [app, masterLabel](float value)
                {
                    app->SetMasterVolume(value);
                    masterLabel->SetText(std::format("{:.0f}%", value * 100));
                }, "master_slider"),
            UI::Anchor::TOP,
            { 200.0f, topY + 200.f }
        );
        UI::Factory::Place(
            ui.CreateText(generalGroup, "Master", 0.0f, "master_text"),
            UI::Anchor::TOP_LEFT,
            { 600.0f, topY + 200.0f }
        )->SetAlignVertical(UI::AlignVertical::CENTRE);

        float soundVolume = app->GetSoundVolume();
        UITextComponent *soundLabel = UI::Factory::Place(
            ui.CreateText(generalGroup, std::format("{:.0f}%", soundVolume * 100), 0.0f, "sound_label"),
            UI::Anchor::TOP,
            { -60.0f, topY + 250.0f }
        );
        soundLabel->SetAlignVertical(UI::AlignVertical::CENTRE);
        soundLabel->SetAlignHorizontal(UI::AlignHorizontal::RIGHT);
        UISliderComponent *soundDrag = UI::Factory::Place(
            ui.CreateSlider(generalGroup, 0.0f, 2.0f, soundVolume, [app, soundLabel](float value)
                {
                    app->SetSoundVolume(value);
                    soundLabel->SetText(std::format("{:.0f}%", value * 100));
                }, "sound_slider"),
            UI::Anchor::TOP,
            { 200.0f, topY + 250.f }
        );
        UI::Factory::Place(
            ui.CreateText(generalGroup, "Sound", 0.0f, "sound_text"),
            UI::Anchor::TOP_LEFT,
            { 600.0f, topY + 250.0f }
        )->SetAlignVertical(UI::AlignVertical::CENTRE);

        float musicVolume = app->GetMusicVolume();
        UITextComponent *musicLabel = UI::Factory::Place(
            ui.CreateText(generalGroup, std::format("{:.0f}%", musicVolume * 100), 0.0f, "music_label"),
            UI::Anchor::TOP,
            { -60.0f, topY + 300.0f }
        );
        musicLabel->SetAlignVertical(UI::AlignVertical::CENTRE);
        musicLabel->SetAlignHorizontal(UI::AlignHorizontal::RIGHT);
        UISliderComponent *musicDrag = UI::Factory::Place(
            ui.CreateSlider(generalGroup, 0.0f, 2.0f, musicVolume, [app, musicLabel](float value)
                {
                    app->SetMusicVolume(value);
                    musicLabel->SetText(std::format("{:.0f}%", value * 100));
                }, "music_slider"),
            UI::Anchor::TOP,
            { 200.0f, topY + 300.0f }
        );
        UI::Factory::Place(
            ui.CreateText(generalGroup, "Music", 0.0f, "music_text"),
            UI::Anchor::TOP_LEFT,
            { 600.0f, topY + 300.0f }
        )->SetAlignVertical(UI::AlignVertical::CENTRE);

        //Controls
        UI::Factory::Place(
            ui.CreateTitle(generalGroup, "Controls", "controls_title"),
            UI::Anchor::TOP,
            { 0.0f, topY + 375.0f }
        )->SetAlignVertical(UI::AlignVertical::CENTRE);

        float initialSens = app ? app->GetMouseSensitivity() : 0.1f;
        UITextComponent *sensLabel = UI::Factory::Place(
            ui.CreateText(generalGroup, std::format("{:.2f}", initialSens), 0.0f, "sens_label"),
            UI::Anchor::TOP,
            { -60.0f, topY + 450.0f }
        );
        sensLabel->SetAlignVertical(UI::AlignVertical::CENTRE);
        sensLabel->SetAlignHorizontal(UI::AlignHorizontal::RIGHT);
        UISliderComponent *sensSlider = UI::Factory::Place(
            ui.CreateSlider(generalGroup, 0.01f, 0.50f, initialSens, [app, sensLabel](float value)
                {
                    if (app)
                        app->SetMouseSensitivity(value);
                    sensLabel->SetText(std::format("{:.2f}", value));
                }, "sens_slider"),
            UI::Anchor::TOP,
            { 200.0f, topY + 450.0f }
        );
        sensSlider->SetRange(0.01f, 0.50f, 0.01f);
        UI::Factory::Place(
            ui.CreateText(generalGroup, "Sensitivity", 28.0f, "sens_text"),
            UI::Anchor::TOP_LEFT,
            { 600.0f, topY + 450.0f }
        )->SetAlignVertical(UI::AlignVertical::CENTRE);

        UICheckboxComponent *invertYCheckbox = UI::Factory::Place(
            ui.CreateCheckbox(generalGroup, "", app ? app->GetInvertY() : false, [app](bool isOn) {
                if (app)
                    app->SetInvertY(isOn);
                }, "y_axis_checkbox"),
            UI::Anchor::TOP,
            { 200.0f, topY + 516.0f }
        );
        UI::Factory::Place(
            ui.CreateText(generalGroup, "Invert Y-Axis", 0.0f, "y_axis_label"),
            UI::Anchor::TOP_LEFT,
            { 600.0f, topY + 520.0f }
        )->SetAlignVertical(UI::AlignVertical::CENTRE);

        return generalBtns;
    }

    std::vector<UIButtonComponent *> CreateKeybindsTab(UI::Factory ui, AppContext appContext, Entity *keybindsGroup, SettingsRebindComponent *rebindComp, float headerY)
    {
        std::vector<UIButtonComponent *> keybindsBtns{};

        UITextComponent *hAction = ui.CreateText(keybindsGroup, "ACTION", 24.0f, "h_action");
        UI::Factory::Place(hAction, UI::Anchor::TOP, { -360.0f, headerY }, { 220.0f, 30.0f });
        hAction->SetAlignHorizontal(UI::AlignHorizontal::LEFT);
        hAction->SetAlignVertical(UI::AlignVertical::CENTRE);
        hAction->SetColour(UI::RGBA{ 243, 187, 130, 255 });
        hAction->SetOutline(2.0f, UI::RGBA{ 40, 20, 10, 255 });

        UITextComponent *hInput1 = ui.CreateText(keybindsGroup, "INPUT 1", 24.0f, "h_input1");
        UI::Factory::Place(hInput1, UI::Anchor::TOP, { -110.0f, headerY }, { 160.0f, 30.0f });
        hInput1->SetAlignHorizontal(UI::AlignHorizontal::CENTRE);
        hInput1->SetAlignVertical(UI::AlignVertical::CENTRE);
        hInput1->SetColour(UI::RGBA{ 243, 187, 130, 255 });
        hInput1->SetOutline(2.0f, UI::RGBA{ 40, 20, 10, 255 });

        UITextComponent *hInput2 = ui.CreateText(keybindsGroup, "INPUT 2", 24.0f, "h_input2");
        UI::Factory::Place(hInput2, UI::Anchor::TOP, { 110.0f, headerY }, { 160.0f, 30.0f });
        hInput2->SetAlignHorizontal(UI::AlignHorizontal::CENTRE);
        hInput2->SetAlignVertical(UI::AlignVertical::CENTRE);
        hInput2->SetColour(UI::RGBA{ 243, 187, 130, 255 });
        hInput2->SetOutline(2.0f, UI::RGBA{ 40, 20, 10, 255 });

        UITextComponent *hCtrl = ui.CreateText(keybindsGroup, "CONTROLLER INPUT", 24.0f, "h_ctrl");
        UI::Factory::Place(hCtrl, UI::Anchor::TOP, { 360.0f, headerY }, { 200.0f, 30.0f });
        hCtrl->SetAlignHorizontal(UI::AlignHorizontal::CENTRE);
        hCtrl->SetAlignVertical(UI::AlignVertical::CENTRE);
        hCtrl->SetColour(UI::RGBA{ 243, 187, 130, 255 });
        hCtrl->SetOutline(2.0f, UI::RGBA{ 40, 20, 10, 255 });

        const std::vector<std::string> actionsToDisplay = {
            "MoveForward",
            "MoveBackward",
            "MoveLeft",
            "MoveRight",
            "Dash",
            "LightAttack",
            "HeavyAttack",
            "Parry"
        };

        const float startRowY = headerY + 50.0f;
        const float rowSpacing = 48.0f;
        ui.style.fontSizeRegular = 22.0f;

        for (size_t i = 0; i < actionsToDisplay.size(); ++i) {
            const std::string &actionName = actionsToDisplay[i];
            const float rowY = startRowY + static_cast<float>(i) * rowSpacing;

            UITextComponent *actionLabel = ui.CreateText(keybindsGroup, Action::GetActionDisplayName(actionName), 22.0f, actionName + "_label");
            UI::Factory::Place(actionLabel, UI::Anchor::TOP, { -360.0f, rowY }, { 220.0f, 36.0f });
            actionLabel->SetAlignHorizontal(UI::AlignHorizontal::LEFT);
            actionLabel->SetAlignVertical(UI::AlignVertical::CENTRE);
            actionLabel->SetColour(UI::RGBA{ 255, 240, 220, 255 });
            actionLabel->SetOutline(2.0f, UI::RGBA{ 30, 20, 15, 255 });

            ActionBinding *binding = Action::GetBinding(actionName);
            std::string text1 = binding ? Action::GetSlotDisplayName(binding->input1) : "---";
            std::string text2 = binding ? Action::GetSlotDisplayName(binding->input2) : "---";
            std::string textCtrl = binding ? Action::GetGamepadButtonDisplayName(binding->controllerInput) : "---";

            UIButtonComponent *btn1 = UI::Factory::Place(
                ui.CreateButton(keybindsGroup, text1, nullptr, actionName + "_btn1"),
                UI::Anchor::TOP,
                { -110.0f, rowY },
                { 160.0f, 36.0f }
            );
            UITextComponent *label1 = UI::Factory::FindLabel(btn1->GetOwner());

            UIButtonComponent *btn2 = UI::Factory::Place(
                ui.CreateButton(keybindsGroup, text2, nullptr, actionName + "_btn2"),
                UI::Anchor::TOP,
                { 110.0f, rowY },
                { 160.0f, 36.0f }
            );
            UITextComponent *label2 = UI::Factory::FindLabel(btn2->GetOwner());

            UIButtonComponent *btnCtrl = UI::Factory::Place(
                ui.CreateButton(keybindsGroup, textCtrl, nullptr, actionName + "_btn_ctrl"),
                UI::Anchor::TOP,
                { 360.0f, rowY },
                { 200.0f, 36.0f }
            );
            UITextComponent *labelCtrl = UI::Factory::FindLabel(btnCtrl->GetOwner());

            btn1->GetOnClick().Connect([rebindComp, actionName, btn1, label1]() {
                rebindComp->StartRebinding(actionName, SettingsRebindComponent::Column::INPUT1, btn1, label1);
                });

            btn2->GetOnClick().Connect([rebindComp, actionName, btn2, label2]() {
                rebindComp->StartRebinding(actionName, SettingsRebindComponent::Column::INPUT2, btn2, label2);
                });

            btnCtrl->GetOnClick().Connect([rebindComp, actionName, btnCtrl, labelCtrl]() {
                rebindComp->StartRebinding(actionName, SettingsRebindComponent::Column::CONTROLLER, btnCtrl, labelCtrl);
                });

            SettingsRebindComponent::RowElements row;
            row.actionName = actionName;
            row.btnInput1 = btn1;
            row.labelInput1 = label1;
            row.btnInput2 = btn2;
            row.labelInput2 = label2;
            row.btnController = btnCtrl;
            row.labelController = labelCtrl;
            rebindComp->AddRow(row);

            keybindsBtns.push_back(btn1);
            keybindsBtns.push_back(btn2);
            keybindsBtns.push_back(btnCtrl);
        }

        UITextComponent *hintText = ui.CreateText(keybindsGroup, "Click any slot to rebind. Press ESC to cancel, Backspace/Del to unbind.", 18.0f, "hint_text");
        UI::Factory::Place(hintText, UI::Anchor::TOP, { 0.0f, startRowY + 400.0f });
        hintText->SetAlignHorizontal(UI::AlignHorizontal::CENTRE);
        hintText->SetColour(UI::RGBA{ 190, 175, 160, 220 });

        ui.style.fontSizeRegular = 22.0f;
        UIButtonComponent *btnReset = UI::Factory::Place(
            ui.CreateButton(keybindsGroup, "RESET DEFAULTS", nullptr, "btn_reset_defaults"),
            UI::Anchor::TOP,
            { 0.0f, startRowY + 440.0f },
            { 240.0f, 40.0f }
        );
        btnReset->GetOnClick().Connect([rebindComp]() {
            rebindComp->CancelRebinding();
            Action::ResetToDefaults();
            Action::SaveBindings("bindings.cfg");
            rebindComp->RefreshAllLabels();
        });

        return keybindsBtns;
    }

    void Settings(Scene &outScene, const AppContext &appContext) {
        std::shared_ptr<Font> font = appContext.assetManager->Get<Font>("assets/fonts/YoungSerif-Regular.ttf");

        UI::Factory ui(outScene, font);
        SetDefaultStyle(ui.style, *appContext.assetManager);

        Entity *canvas = ui.CreateCanvas();
        canvas->AddComponent<TransformComponent>();
        canvas->AddComponent<CameraComponent>();
        canvas->AddComponent<MenuScreenComponent>();
        canvas->AddComponent<EnvironmentComponent>();

        SettingsRebindComponent *rebindComp = canvas->AddComponent<SettingsRebindComponent>();

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
        UI::Factory::Place(titleText, UI::Anchor::TOP, {0.0f, 40.0f});
        titleText->SetAlignHorizontal(UI::AlignHorizontal::CENTRE);
        titleText->SetColour(UI::RGBA{25, 17, 6, 255});
        titleText->SetLineSpacing(0.8f);
        titleText->SetOutline(7.0f, UI::RGBA{103, 44, 32, 255});
        titleText->SetShadow({8.0f, 8.0f}, UI::RGBA{0, 0, 0, 136});

        // Tabs
        float tabButtonY = 200.0f;

        ui.style.fontSizeRegular = 24.0f;

        UIButtonComponent *btnTabGeneral = UI::Factory::Place(
            ui.CreateButton(bgEntity, "GENERAL", nullptr, "btn_tab_general"),
            UI::Anchor::TOP,
            { -140.0f, tabButtonY },
            { 240.0f, 44.0f }
        );

        UIButtonComponent *btnTabKeybinds = UI::Factory::Place(
            ui.CreateButton(bgEntity, "KEYBINDS", nullptr, "btn_tab_keybinds"),
            UI::Anchor::TOP,
            { 140.0f, tabButtonY },
            { 240.0f, 44.0f }
        );

        Entity *generalGroup = ui.CreateGroup(bgEntity, { 0.0f, 0.0f }, "general_group");
        UI::Factory::Place(generalGroup, UI::Anchor::STRETCH, { 0.0f, 0.0f });

        Entity *keybindsGroup = ui.CreateGroup(bgEntity, { 0.0f, 0.0f }, "keybinds_group");
        UI::Factory::Place(keybindsGroup, UI::Anchor::STRETCH, { 0.0f, 0.0f });

        const float tabContentTopY = tabButtonY + 60.0f;
        // General settings
        std::vector<UIButtonComponent *> generalSettngsBtns = CreateGeneralSettingsTab(ui, appContext, generalGroup, tabContentTopY);

        // Keybinds Table
        std::vector<UIButtonComponent *> keybindsBtns = CreateKeybindsTab(ui, appContext, keybindsGroup, rebindComp, tabContentTopY);

        UIButtonComponent *backButton = UI::Factory::Place(ui.CreateButton(bgEntity, "BACK", nullptr, "btn_back"), UI::Anchor::BOTTOM, { 0.0f, -80.0f });
        backButton->GetOnClick().Connect([appContext, rebindComp]()
        {
            rebindComp->CancelRebinding();
            appContext.sceneManager->RequestSceneChange("main_menu");
        });

        const DirectX::XMFLOAT4 tabActiveColour = {0.8f, 0.22f, 0.24f, 1.0f};
        const DirectX::XMFLOAT4 tabActiveHoverColour = {0.88f, 0.30f, 0.32f, 1.0f};
        const DirectX::XMFLOAT4 tabInactiveColour = {0.45f, 0.35f, 0.32f, 1.0f};
        const DirectX::XMFLOAT4 tabInactiveHoverColour = {0.65f, 0.50f, 0.45f, 1.0f};

        auto updateTabs = [=](bool showKeybinds) {
            keybindsGroup->SetLocallyActive(showKeybinds);
            generalGroup->SetLocallyActive(!showKeybinds);

            btnTabKeybinds->SetNormalColour(showKeybinds ? tabActiveColour : tabInactiveColour);
            btnTabKeybinds->SetHoveredColour(showKeybinds ? tabActiveHoverColour : tabInactiveHoverColour);

            btnTabGeneral->SetNormalColour(!showKeybinds ? tabActiveColour : tabInactiveColour);
            btnTabGeneral->SetHoveredColour(!showKeybinds ? tabActiveHoverColour : tabInactiveHoverColour);

            if (!showKeybinds) {
                rebindComp->CancelRebinding();
            }
        };

        btnTabKeybinds->GetOnClick().Connect([updateTabs]() {
            updateTabs(true);
        });

        btnTabGeneral->GetOnClick().Connect([updateTabs]() {
            updateTabs(false);
        });

        // General tab open by default
        updateTabs(false);

        std::vector<UIButtonComponent*> allButtons = {
            btnTabKeybinds, btnTabGeneral, backButton
        };
        for (UIButtonComponent *b : generalSettngsBtns) {
            allButtons.push_back(b);
        }
        for (UIButtonComponent* b : keybindsBtns) {
            allButtons.push_back(b);
        }
        SetDefaultSounds(outScene, appContext, allButtons);

        SetDefaultAmbientSound(outScene, appContext);
    }
}
