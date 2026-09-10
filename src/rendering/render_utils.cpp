#include "render_utils.hpp"

namespace RenderUtils {
    RenderView *GetView(std::vector<RenderView> &views, ViewType type, int index) {
        for (RenderView &view : views)
            if (view.type == type && view.index == index)
                return &view;

        return nullptr;
    }
}
