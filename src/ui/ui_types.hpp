#pragma once

#include <DirectXMath.h>

namespace UI {
    struct Rect {
        DirectX::XMFLOAT2 topLeft = {0.0f, 0.0f};
        DirectX::XMFLOAT2 size = {0.0f, 0.0f};

        DirectX::XMFLOAT2 BottomRight() const { return {topLeft.x + size.x, topLeft.y + size.y}; }

        bool Contains(DirectX::XMFLOAT2 p) const {
            return p.x >= topLeft.x && p.y >= topLeft.y && p.x < this->BottomRight().x && p.y < this->BottomRight().y;
        }
    };

    enum class Anchor : int {
         TOP_LEFT,    TOP,    TOP_RIGHT,
           LEFT,     CENTRE,    RIGHT,
        BOTTOM_LEFT, BOTTOM, BOTTOM_RIGHT,

        STRETCHED_HORIZONTAL_TOP,
        STRETCHED_HORIZONTAL_MIDDLE,
        STRETCHED_HORIZONTAL_BOTTOM,

        STRETCHED_VERTICAL_LEFT,
        STRETCHED_VERTICAL_MIDDLE,
        STRETCHED_VERTICAL_RIGHT,

        STRETCH,

        COUNT
    };

    enum class AlignHorizontal {
        LEFT,
        CENTRE,
        RIGHT
    };

    enum class AlignVertical {
        TOP,
        CENTRE,
        BOTTOM
    };
}