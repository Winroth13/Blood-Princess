#pragma once

#include "rendering/render_view.hpp"

#include <vector>

#define SafeRelease(obj) do { if (obj) (obj)->Release(); (obj) = nullptr; } while (0)

namespace RenderUtils {
    RenderView *GetView(std::vector<RenderView> &views, ViewType type, int index = 0); 
}
