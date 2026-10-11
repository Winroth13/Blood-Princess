#include "ui_context.hpp"

namespace UI {
    void Context::BeginFrame(uint32_t width, uint32_t height) {
        this->drawList.Clear();
        ++this->frameIndex;
        this->width = width;
        this->height = height;
    }

    void Context::Clear() {
        this->drawList.Clear();
        ++this->frameIndex;
    }
}
