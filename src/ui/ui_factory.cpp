#include "ui_factory.hpp"
#include "scene/scene.hpp"
#include "scene/entity.hpp"
#include "scene/component.hpp"

#include "components/ui/ui_bar_component.hpp"
#include "components/ui/ui_button_component.hpp"
#include "components/ui/ui_canvas_component.hpp"
#include "components/ui/ui_checkbox_component.hpp"
#include "components/ui/ui_image_component.hpp"
#include "components/ui/ui_interactable_component.hpp"
#include "components/ui/ui_rect_component.hpp"
#include "components/ui/ui_slider_component.hpp"
#include "components/ui/ui_text_component.hpp"

#undef FindText
#undef min
#undef max

using namespace DirectX;

namespace UI {
    Entity *Factory::CreateEntity(Entity *parent, const std::string &name, DirectX::XMFLOAT2 sizeDelta) {
        Entity *entity = this->scene.AddEntity(name);
        entity->SetParent(parent);
        entity->AddComponent<UIRectComponent>()->Place(UI::Anchor::CENTRE, {0.0f, 0.0f}, sizeDelta);
        return entity;
    }

    UITextComponent *Factory::AddText(Entity *entity, const std::string &text, float fontSize) {
        UITextComponent *textComponent = entity->AddComponent<UITextComponent>();
        textComponent->SetText(text);
        textComponent->SetFontSize(fontSize);
        textComponent->SetColour(this->style.textColour);
        textComponent->SetAlignment(UI::AlignHorizontal::LEFT, UI::AlignVertical::TOP);

        if (this->style.font)
            textComponent->SetFont(this->style.font);

        if (this->style.textOutlineThickness > 0.0f)
            textComponent->SetOutline(this->style.textOutlineThickness, this->style.textOutlineColour);

        return textComponent;
    }

    UIButtonComponent *Factory::AddButton(Entity *entity, UIImageComponent *targetImage, UITextComponent *label) {
        UIButtonComponent *buttonComponent = entity->AddComponent<UIButtonComponent>();
        buttonComponent->SetTargetImage(targetImage);
        buttonComponent->SetColours(
            this->style.buttonColourNormal, 
            this->style.buttonColourHovered, 
            this->style.buttonColourPressed, 
            this->style.buttonColourDisabled
        );
        buttonComponent->SetFadeDuration(this->style.buttonFadeDuration);

        buttonComponent->SetHoveredTexture(this->style.buttonTextureHovered);
        buttonComponent->SetPressedTexture(this->style.buttonTexturePressed);
        buttonComponent->SetDisabledTexture(this->style.buttonTextureDisabled);

        if (label) {
            const XMFLOAT4 textColour = this->style.textColour;
            const XMFLOAT4 textColourDisabled = this->style.textColourDisabled;

            buttonComponent->GetOnStateChanged().Connect([label, textColour, textColourDisabled](UI::ButtonState state) {
                label->SetColour(state == UI::ButtonState::DISABLED ? textColourDisabled : textColour);
            });
        }

        return buttonComponent;
    }

    void Factory::PlaceEntity(Entity *entity, UI::Anchor anchor, DirectX::XMFLOAT2 position, const DirectX::XMFLOAT2 *sizeDelta) {
        UIRectComponent *rectComponent = Factory::GetRect(entity);
        if (!rectComponent)
            return;

        rectComponent->SetAnchorPreset(anchor);
        rectComponent->SetPosition(position);

        if (sizeDelta)
            rectComponent->SetSizeDelta(*sizeDelta);
    }

    void Factory::SetEntitySizeDelta(Entity *entity, DirectX::XMFLOAT2 sizeDelta) {
        if (UIRectComponent *rectComponent = Factory::GetRect(entity))
            rectComponent->SetSizeDelta(sizeDelta);
    }

    Entity *Factory::CreateCanvas(const std::string &name) {
        Entity *canvas = this->scene.AddEntity(name);
        canvas->AddComponent<UIRectComponent>();
        canvas->AddComponent<UICanvasComponent>();
        return canvas;
    }

    Entity *Factory::CreateGroup(Entity *parent, DirectX::XMFLOAT2 sizeDelta, const std::string &name) {
        return this->CreateEntity(parent, name, sizeDelta);
    }

    Entity *Factory::CreatePanel(Entity *parent, DirectX::XMFLOAT2 sizeDelta, const std::string &name) {
        Entity *panel = this->CreateEntity(parent, name, sizeDelta);

        UIImageComponent *image = panel->AddComponent<UIImageComponent>();
        image->SetTexture(this->style.panelTexture);
        image->SetColour(this->style.panelColour);
        image->SetSlicing(this->style.panelSliceBorder);

        return panel;
    }

    Entity *Factory::CreateBlocker(Entity *parent, const std::string &name) {
        Entity *blocker = this->CreateEntity(parent, name, {0.0f, 0.0f});
        Factory::Place(blocker, UI::Anchor::STRETCH, {0.0f, 0.0f});

        blocker->AddComponent<UIImageComponent>()->SetColour(this->style.blockerColour);
        return blocker;
    }

    UIImageComponent *Factory::CreateImage(Entity *parent, std::shared_ptr<Texture2D> texture, const std::string &name) {
        const XMFLOAT2 size = texture ? XMFLOAT2(texture->GetWidth(), texture->GetHeight()) : XMFLOAT2(100.0f, 100.0f);
        UIImageComponent *image = this->CreateEntity(parent, name, size)->AddComponent<UIImageComponent>();
        image->SetTexture(texture);
        return image;
    }

    UITextComponent *Factory::CreateText(Entity *parent, const std::string &text, float fontSize, const std::string &name) {
        if (fontSize <= 0.0f)
            fontSize = this->style.fontSizeRegular;

        Entity *entity = this->CreateEntity(parent, name, {100.0f, fontSize * 1.5f});
        return Factory::FitToText(this->AddText(entity, text, fontSize));
    }

    UITextComponent *Factory::CreateTitle(Entity *parent, const std::string &text, const std::string &name) {
        UITextComponent *title = this->CreateText(parent, text, this->style.fontSizeTitle, name);
        title->SetAlignment(UI::AlignHorizontal::CENTRE, UI::AlignVertical::TOP);
        return title;
    }

    UIButtonComponent *Factory::CreateButton(
        Entity *parent,
        const std::string &text,
        std::function<void()> onClick,
        const std::string &name
    ) {
        Entity *entity = this->CreateEntity(parent, name, {0.0f, 0.0f});

        UIImageComponent *image = entity->AddComponent<UIImageComponent>();
        image->SetTexture(this->style.buttonTextureNormal);
        image->SetSlicing(this->style.buttonSliceBorder);

        UITextComponent *label = this->CreateText(entity, text, 0.0f, "label");
        label->SetAlignment(UI::AlignHorizontal::CENTRE, UI::AlignVertical::CENTRE);

        UIButtonComponent *button = this->AddButton(entity, image, label);
        this->FitButtonToText(button);

        if (onClick)
            button->GetOnClick().Connect(std::move(onClick));

        return button;
    }

    UICheckboxComponent *Factory::CreateCheckbox(
        Entity *parent,
        const std::string &text,
        bool isOn,
        std::function<void(bool)> onValueChanged,
        const std::string &name
    ) {
        const float boxSize = this->style.checkboxSize;
        const float labelOffset = boxSize + this->style.checkboxSpacing;

        Entity *row = this->CreateEntity(parent, name, {0.0f, 0.0f});
        row->AddComponent<UIImageComponent>()->SetColour({0.0f, 0.0f, 0.0f, 0.0f});

        Entity *box = this->CreateEntity(row, "box", {boxSize, boxSize});
        Factory::Place(box, UI::Anchor::LEFT, {0.0f, 0.0f});
        UIImageComponent *boxImage = box->AddComponent<UIImageComponent>();
        boxImage->SetTexture(this->style.buttonTextureNormal);
        boxImage->SetSlicing(this->style.buttonSliceBorder);

        Entity *checkmark = this->CreateEntity(box, "checkmark", {0.0f, 0.0f});
        Factory::Place(checkmark, UI::Anchor::STRETCH, {0.0f, 0.0f}, {-boxSize * 0.3f, -boxSize * 0.3f});
        UIImageComponent *checkmarkImage = checkmark->AddComponent<UIImageComponent>();
        checkmarkImage->SetTexture(this->style.checkmarkTexture);
        checkmarkImage->SetColour(this->style.checkmarkColour);
        checkmarkImage->SetRaycastTarget(false);

        UITextComponent *label = this->CreateText(row, text, 0.0f, "label");
        Factory::Place(label, UI::Anchor::LEFT, {labelOffset, 0.0f});

        const XMFLOAT2 labelSize = Factory::GetRect(label->GetOwner())->GetSizeDelta();
        Factory::SetSizeDelta(row, {labelOffset + labelSize.x, std::max(boxSize, labelSize.y)});

        UICheckboxComponent *checkbox = row->AddComponent<UICheckboxComponent>();
        checkbox->SetCheckMark(checkmark);
        checkbox->SetOn(isOn, false);

        this->AddButton(row, boxImage, label);

        if (onValueChanged)
            checkbox->GetOnValueChanged().Connect(std::move(onValueChanged));

        return checkbox;
    }

    UISliderComponent *Factory::CreateSlider(
        Entity *parent,
        float minValue,
        float maxValue,
        float value,
        std::function<void(float)> onValueChanged,
        const std::string &name
    ) {
        Entity *entity = this->CreateEntity(parent, name, this->style.sliderSize);

        UIBarComponent *bar = entity->AddComponent<UIBarComponent>();
        bar->SetBackgroundTexture(this->style.panelTexture);
        bar->SetBackgroundSliceBorder(this->style.panelSliceBorder);
        bar->SetBackgroundColour(this->style.panelColour);
        bar->SetFillTexture(this->style.buttonTextureNormal);
        bar->SetFillSliceBorder(this->style.buttonSliceBorder);
        bar->SetFillColour(this->style.sliderFillColour);
        bar->SetTrailColour({0.0f, 0.0f, 0.0f, 0.0f});
        bar->SetSliceScale(this->style.sliderSliceScale);
        bar->SetFillPadding(this->style.sliderPadding);

        Entity *handle = this->CreateEntity(entity, "handle", this->style.sliderHandleSize);
        UIImageComponent *handleImage = handle->AddComponent<UIImageComponent>();
        handleImage->SetTexture(this->style.buttonTextureNormal);
        handleImage->SetSlicing(this->style.buttonSliceBorder, this->style.sliderSliceScale);

        UISliderComponent *slider = entity->AddComponent<UISliderComponent>();
        slider->SetHandle(handle);
        slider->SetRange(minValue, maxValue);
        slider->SetValue(value, false);

        this->AddButton(entity, handleImage, nullptr)->SetStaysPressedWhenDraggedOff(true);

        if (onValueChanged)
            slider->GetOnValueChanged().Connect(std::move(onValueChanged));

        return slider;
    }

    UIBarComponent *Factory::CreateBar(
        Entity *parent,
        float value,
        float maxValue,
        const std::string &name
    ) {
        UIBarComponent *bar = this->CreateEntity(parent, name, this->style.barSize)->AddComponent<UIBarComponent>();
        bar->SetBackgroundTexture(this->style.panelTexture);
        bar->SetBackgroundSliceBorder(this->style.panelSliceBorder);
        bar->SetBackgroundColour(this->style.panelColour);
        bar->SetFillTexture(this->style.barFillTexture);
        bar->SetFillSliceBorder(this->style.barFillSliceBorder);
        bar->SetFillColour(this->style.barColourFill);
        bar->SetTrailColour(this->style.barColourTrail);
        bar->SetSliceScale(this->style.barSliceScale);
        bar->SetFillPadding(this->style.barPadding);
        bar->SetValue(value, maxValue);
        return bar;
    }

    UIButtonComponent *Factory::FitButtonToText(UIButtonComponent *button) const {
        if (!button)
            return button;

        UITextComponent *label = Factory::FindLabel(button->GetOwner());
        if (!label)
            return button;

        Factory::FitToText(label);

        const XMFLOAT2 labelSize = Factory::GetRect(label->GetOwner())->GetSizeDelta();
        Factory::SetSizeDelta(button, {
            std::max(labelSize.x + 2.0f * this->style.buttonTextPadding.x, this->style.buttonMinSize.x),
            std::max(labelSize.y + 2.0f * this->style.buttonTextPadding.y, this->style.buttonMinSize.y)
        });

        return button;
    }

    Entity *Factory::GetEntity(Component *component) {
        return component ? component->GetOwner() : nullptr;
    }

    UIRectComponent *Factory::GetRect(Entity *entity) {
        return entity ? entity->GetComponent<UIRectComponent>() : nullptr;
    }

    UITextComponent *Factory::FindLabel(Entity *entity) {
        if (!entity)
            return nullptr;

        if (UITextComponent *textComponent = entity->GetComponent<UITextComponent>())
            return textComponent;

        for (Entity *child : entity->GetChildren())
            if (UITextComponent *textComponent = Factory::FindLabel(child))
                return textComponent;

        return nullptr;
    }

    UITextComponent *Factory::FitToText(UITextComponent *text, float maxWidth) {
        if (!text)
            return text;

        const std::shared_ptr<Font> font = text->GetFont();
        if (!font) {
            text->SetShouldWrap(maxWidth > 0.0f);
            return text;
        }

        const float lineHeight = UI::TextLayout::GetSingleLineHeight(*font, text->GetFontSize());

        if (maxWidth > 0.0f) {
            text->SetShouldWrap(true);
            const float height = text->GetPreferredSize(std::max(1.0f, maxWidth - 1.0f)).y;
            Factory::SetEntitySizeDelta(text->GetOwner(), {maxWidth, std::ceil(std::max(height, lineHeight))});
        }
        else {
            text->SetShouldWrap(false);
            const XMFLOAT2 size = text->GetPreferredSize();
            Factory::SetEntitySizeDelta(text->GetOwner(), {std::ceil(size.x), std::ceil(std::max(size.y, lineHeight))});
        }

        return text;
    }
}