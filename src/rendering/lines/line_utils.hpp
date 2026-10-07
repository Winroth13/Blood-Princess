#pragma once
#include "line_vertex.hpp"

#include <vector>
#include <DirectXMath.h>
#include <DirectXCollision.h>

using namespace DirectX;

namespace LineUtils
{
	void GetBoxVertices(std::vector<LineVertex> &vertices, const BoundingBox &boundingBox, XMMATRIX transform, XMFLOAT3 color);

	void GetBoxVertices(std::vector<LineVertex> &vertices, const BoundingOrientedBox &obb, XMFLOAT3 color);
}