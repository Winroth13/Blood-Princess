#pragma once

#include "ui/ui_style.hpp"
#include "ui/ui_types.hpp"

#include <string>
#include <DirectXMath.h>
#include <memory>
#include <functional>

class Scene;
class Entity;
class Component;

class AssetManager;
class Font;

class UIBarComponent;
class UIButtonComponent;
class UICanvasComponent;
class UICheckboxComponent;
class UIImageComponent;
class UIInteractableComponent;
class UIRectComponent;
class UISliderComponent;
class UITextComponent;

namespace UI {
    class Factory {
        Scene &scene;

        Entity *CreateEntity(Entity *parent, const std::string &name, DirectX::XMFLOAT2 sizeDelta);

        UITextComponent *AddText(Entity *entity, const std::string &text, float fontSize);
        UIButtonComponent *AddButton(Entity *entity, UIImageComponent *targetImage, UITextComponent *label);

        static void PlaceEntity(Entity *entity, UI::Anchor anchor, DirectX::XMFLOAT2 position, const DirectX::XMFLOAT2 *sizeDelta);
        static void SetEntitySizeDelta(Entity *entity, DirectX::XMFLOAT2 sizeDelta);

    public:
        Style style;

        Factory(Scene &scene, AssetManager &assetManager) : scene(scene), style(assetManager) {}
        Factory(Scene &scene, std::shared_ptr<Font> font) : scene(scene), style(font) {}
        Factory(Scene &scene, const Style &style) : scene(scene), style(style) {}

        Entity *CreateCanvas(const std::string &name = "canvas");
        Entity *CreateGroup(Entity *parent, DirectX::XMFLOAT2 sizeDelta, const std::string &name = "group");
        Entity *CreatePanel(Entity *parent, DirectX::XMFLOAT2 sizeDelta, const std::string &name = "panel");
        Entity *CreateBlocker(Entity *parent, const std::string &name = "blocker");

        UIImageComponent *CreateImage(Entity *parent, std::shared_ptr<Texture2D> texture = nullptr, const std::string &name = "image");

        UITextComponent *CreateText(Entity *parent, const std::string &text, float fontSize = 0.0f, const std::string &name = "text");
        UITextComponent *CreateTitle(Entity *parent, const std::string &text, const std::string &name = "title");

        UIButtonComponent *CreateButton(
            Entity *parent, 
            const std::string &text, 
            std::function<void()> onClick = nullptr, 
            const std::string &name = "button"
        );

        UICheckboxComponent *CreateCheckbox(
            Entity *parent, 
            const std::string &text, 
            bool isOn,
            std::function<void(bool)> onValueChanged = nullptr,
            const std::string &name = "checkbox"
        );

        UISliderComponent *CreateSlider(
            Entity *parent, 
            float minValue,
            float maxValue,
            float value,
            std::function<void(float)> onValueChanged = nullptr,
            const std::string &name = "slider"
        );

        UIBarComponent *CreateBar(
            Entity *parent, 
            float value,
            float maxValue,
            const std::string &name = "bar"
        );

        template <typename T>
        static T *Place(T *element, UI::Anchor anchor, DirectX::XMFLOAT2 position) {
            Factory::PlaceEntity(Factory::GetEntity(element), anchor, position, nullptr);
            return element;
        }

        template <typename T>
        static T *Place(T *element, UI::Anchor anchor, DirectX::XMFLOAT2 position, DirectX::XMFLOAT2 sizeDelta) {
            Factory::PlaceEntity(Factory::GetEntity(element), anchor, position, &sizeDelta);
            return element;
        }

        template <typename T>
        static T *SetSizeDelta(T *element, DirectX::XMFLOAT2 sizeDelta) {
            Factory::SetEntitySizeDelta(Factory::GetEntity(element), sizeDelta);
            return element;
        }

        UIButtonComponent *FitButtonToText(UIButtonComponent *button) const;

        static Entity *GetEntity(Entity *entity) { return entity; }
        static Entity *GetEntity(Component *component);

        static UIRectComponent *GetRect(Entity *entity);
        static UITextComponent *FindLabel(Entity *entity);
        static UITextComponent *FitToText(UITextComponent *text, float maxWidth = 0.0f);
    };
}