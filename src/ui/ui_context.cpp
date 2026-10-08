#include "ui_context.hpp"

void UIContext::BeginFrame(uint32_t width, uint32_t height) {
    this->drawList.Clear();
    ++this->frameIndex;
    this->width = width;
    this->height = height;
}

void UIContext::Clear() {
    this->drawList.Clear();
    ++this->frameIndex;
}
