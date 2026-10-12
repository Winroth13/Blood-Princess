#pragma once

#include "scene/component.hpp"
#include "ui/ui_types.hpp"
#include "events/signal.hpp"
#include "assets/types/texture_2d.hpp"

#include <DirectXMath.h>
#include <cstdint>
#include <memory>

using namespace DirectX;

class UIImageComponent;
class UIInteractableComponent;

class UIButtonComponent : public Component {
    RegisterComponent(UIButtonComponent);

    ExportProperty(bool, staysPressedWhenDraggedOff, false);

    ExportPropertyCategory("Tint");
    ExportProperty(bool, isTinted, true);
    ExportPropertyColor(XMFLOAT4, normalColour, {0.85f, 0.85f, 0.85f, 1.0f});
    ExportPropertyColor(XMFLOAT4, hoveredColour, {1.0f, 1.0f, 1.0f, 1.0f});
    ExportPropertyColor(XMFLOAT4, pressedColour, {0.6f, 0.6f, 0.6f, 1.0f});
    ExportPropertyColor(XMFLOAT4, disabledColour, {0.5f, 0.5f, 0.5f, 0.5f});
    ExportPropertyRanged(float, fadeDuration, "s", 0.0f, 2.0f, 0.01f, 0.1f);

    ExportPropertyCategory("Sprite swap");
    ExportProperty(std::shared_ptr<Texture2D>, hoveredTexture, nullptr);
    ExportProperty(std::shared_ptr<Texture2D>, pressedTexture, nullptr);
    ExportProperty(std::shared_ptr<Texture2D>, disabledTexture, nullptr);

    UIImageComponent *targetImage = nullptr;

    UI::ButtonState state = UI::ButtonState::NORMAL;
    Signal<UI::ButtonState> onStateChanged;

    XMFLOAT4 colour = {1.0f, 1.0f, 1.0f, 1.0f};
    XMFLOAT4 fadeFrom = {1.0f, 1.0f, 1.0f, 1.0f};
    float fadeProgress = 1.0f;

    std::shared_ptr<Texture2D> normalTexture = nullptr;
    std::shared_ptr<Texture2D> appliedTexture = nullptr;

    uint32_t lastFrameIndex = 0;
    bool hasShownState = false;

    UI::ButtonState GetStateFromInteractable();
    XMFLOAT4 GetStateColour(UI::ButtonState state) const;
    std::shared_ptr<Texture2D> GetStateTexture(UI::ButtonState state) const;

public:
    UIButtonComponent(Entity *owner, bool isActive) : Component(owner, isActive) { UsesUpdate(); }

private:
    void OnStart(const AppContext &context) override;
    void Update(const AppContext &context, float deltaTime) override;

public:
    UIInteractableComponent *GetInteractable();

    Signal<> &GetOnClick();
    Signal<UI::ButtonState> &GetOnStateChanged() { return this->onStateChanged; }

    UI::ButtonState GetState() const { return this->state; }

    bool IsInteractable();
    void SetInteractable(bool isInteractable);

    UIImageComponent *GetTargetImage();
    void SetTargetImage(UIImageComponent *targetImage);

    bool IsTinted() const { return this->isTinted; }
    void SetTinted(bool isTinted) { this->isTinted = isTinted; }

    XMFLOAT4 GetNormalColour() const { return this->normalColour; }
    void SetNormalColour(XMFLOAT4 normalColour) { this->normalColour = normalColour; }

    XMFLOAT4 GetHoveredColour() const { return this->hoveredColour; }
    void SetHoveredColour(XMFLOAT4 hoveredColour) { this->hoveredColour = hoveredColour; }

    XMFLOAT4 GetPressedColour() const { return this->pressedColour; }
    void SetPressedColour(XMFLOAT4 pressedColour) { this->pressedColour = pressedColour; }

    XMFLOAT4 GetDisabledColour() const { return this->disabledColour; }
    void SetDisabledColour(XMFLOAT4 disabledColour) { this->disabledColour = disabledColour; }

    void SetColours(XMFLOAT4 normalColour, XMFLOAT4 hoveredColour, XMFLOAT4 pressedColour, XMFLOAT4 disabledColour) {
        this->normalColour = normalColour;
        this->hoveredColour = hoveredColour;
        this->pressedColour = pressedColour;
        this->disabledColour = disabledColour;
    }

    float GetFadeDuration() const { return this->fadeDuration; }
    void SetFadeDuration(float fadeDuration) { this->fadeDuration = fadeDuration; }

    bool StaysPressedWhenDraggedOff() const { return this->staysPressedWhenDraggedOff; }
    void SetStaysPressedWhenDraggedOff(bool staysPressedWhenDraggedOff) { this->staysPressedWhenDraggedOff = staysPressedWhenDraggedOff; }

    std::shared_ptr<Texture2D> GetHoveredTexture() const { return this->hoveredTexture; }
    void SetHoveredTexture(std::shared_ptr<Texture2D> hoveredTexture) { this->hoveredTexture = hoveredTexture; }

    std::shared_ptr<Texture2D> GetPressedTexture() const { return this->pressedTexture; }
    void SetPressedTexture(std::shared_ptr<Texture2D> pressedTexture) { this->pressedTexture = pressedTexture; }

    std::shared_ptr<Texture2D> GetDisabledTexture() const { return this->disabledTexture; }
    void SetDisabledTexture(std::shared_ptr<Texture2D> disabledTexture) { this->disabledTexture = disabledTexture; }
};
