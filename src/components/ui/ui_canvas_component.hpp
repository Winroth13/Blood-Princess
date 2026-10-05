#pragma once

#include "scene/component.hpp"

class UICanvasComponent : public Component {
    RegisterComponent(UICanvasComponent);

    ExportProperty(float, virtualHeight, 1080.0f);
    ExportPropertyRanged(float, scale, "x", 0.01f, 10.0f, 0.01f, 1.0f);

public:
    UICanvasComponent(Entity *owner, bool isActive) : Component(owner, isActive) { NoUpdate(), NoLateUpdate(), NoRender(); }

private:
    void OnStart(const AppContext &appContext) override;

public:
    float GetScaleFactor(uint32_t screenHeight) const;

    float GetVirtualHeight() const { return this->virtualHeight; }
    void SetVirtualHeight(float virtualHeight) { this->virtualHeight = virtualHeight; }

    float GetScale() const { return this->scale; }
    void SetScale(float scale) { this->scale = scale; }
};