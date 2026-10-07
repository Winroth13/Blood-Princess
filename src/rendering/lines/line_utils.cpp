#include "line_utils.hpp"

#include <array>

void LineUtils::GetBoxVertices(
    std::vector<LineVertex> &vertices, 
    const BoundingBox &boundingBox,
    XMMATRIX transform,
    XMFLOAT3 color
)
{
    std::array<XMFLOAT3, DirectX::BoundingBox::CORNER_COUNT> cornersf{};
    boundingBox.GetCorners(cornersf.data());

    std::array<XMVECTOR, DirectX::BoundingBox::CORNER_COUNT> corners{};
    for (size_t i = 0; i < cornersf.size(); ++i)
    {
        corners[i] = XMVector3Transform(XMLoadFloat3(&cornersf[i]), transform);
    }

    for (size_t i = 0; i < corners.size(); ++i)
    {
        XMStoreFloat3(&cornersf[i], corners[i]);
    }

    vertices.reserve(vertices.size() + 24);

    vertices.push_back({ cornersf[0], color }); vertices.push_back({ cornersf[1], color });
    vertices.push_back({ cornersf[1], color }); vertices.push_back({ cornersf[2], color });
    vertices.push_back({ cornersf[2], color }); vertices.push_back({ cornersf[3], color });
    vertices.push_back({ cornersf[3], color }); vertices.push_back({ cornersf[0], color });

    vertices.push_back({ cornersf[4], color }); vertices.push_back({ cornersf[5], color });
    vertices.push_back({ cornersf[5], color }); vertices.push_back({ cornersf[6], color });
    vertices.push_back({ cornersf[6], color }); vertices.push_back({ cornersf[7], color });
    vertices.push_back({ cornersf[7], color }); vertices.push_back({ cornersf[4], color });

    vertices.push_back({ cornersf[0], color }); vertices.push_back({ cornersf[4], color });
    vertices.push_back({ cornersf[1], color }); vertices.push_back({ cornersf[5], color });
    vertices.push_back({ cornersf[2], color }); vertices.push_back({ cornersf[6], color });
    vertices.push_back({ cornersf[3], color }); vertices.push_back({ cornersf[7], color });
}

void LineUtils::GetBoxVertices(
    std::vector<LineVertex> &vertices,
    const BoundingOrientedBox &obb,
    XMFLOAT3 color
)
{
    std::array<XMFLOAT3, BoundingOrientedBox::CORNER_COUNT> cornersf = {};
    obb.GetCorners(cornersf.data());

    /*std::array<XMVECTOR, DirectX::BoundingOrientedBox::CORNER_COUNT> corners{};
    for (size_t i = 0; i < cornersf.size(); ++i)
    {
        corners[i] = XMVector3Transform(XMLoadFloat3(&cornersf[i]), transform);
    }*/

    /*for (size_t i = 0; i < corners.size(); ++i)
    {
        XMStoreFloat3(&cornersf[i], corners[i]);
    }*/

    vertices.reserve(vertices.size() + 24);

    vertices.push_back({ cornersf[0], color }); vertices.push_back({ cornersf[1], color });
    vertices.push_back({ cornersf[1], color }); vertices.push_back({ cornersf[2], color });
    vertices.push_back({ cornersf[2], color }); vertices.push_back({ cornersf[3], color });
    vertices.push_back({ cornersf[3], color }); vertices.push_back({ cornersf[0], color });

    vertices.push_back({ cornersf[4], color }); vertices.push_back({ cornersf[5], color });
    vertices.push_back({ cornersf[5], color }); vertices.push_back({ cornersf[6], color });
    vertices.push_back({ cornersf[6], color }); vertices.push_back({ cornersf[7], color });
    vertices.push_back({ cornersf[7], color }); vertices.push_back({ cornersf[4], color });

    vertices.push_back({ cornersf[0], color }); vertices.push_back({ cornersf[4], color });
    vertices.push_back({ cornersf[1], color }); vertices.push_back({ cornersf[5], color });
    vertices.push_back({ cornersf[2], color }); vertices.push_back({ cornersf[6], color });
    vertices.push_back({ cornersf[3], color }); vertices.push_back({ cornersf[7], color });
}