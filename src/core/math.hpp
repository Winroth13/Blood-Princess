#pragma once

#include <DirectXMath.h>
using namespace DirectX;

#include <cmath>

namespace Math
{
	float Lerp(float a, float b, float x);

	XMFLOAT4 Lerp(XMFLOAT4 a, XMFLOAT4 b, float x);

	float easeOutElastic(float x);
	float easeInElastic(float x);
	float easeInOutQuint(float x);
	float easeOutQuint(float x);
}