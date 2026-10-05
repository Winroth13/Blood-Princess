#include "ui_canvas_component.hpp"
#include "components/ui/ui_rect_component.hpp"
#include "scene/entity.hpp"
#include "core/logging.hpp"

void UICanvasComponent::OnStart(const AppContext &appContext) {
    if (!this->GetOwner()->HasComponent<UIRectComponent>())
        LogWarn("Missing UIRectComponent!");
}

float UICanvasComponent::GetScaleFactor(uint32_t screenHeight) const {
    return this->scale * screenHeight / this->virtualHeight;
}