#pragma once

#include <DirectXMath.h>

struct LineVertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 color;

	LineVertex(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 color)
		:position(position), color(color)
	{ }
};

