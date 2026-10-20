#pragma once

#include <cmath>

namespace Math
{
	float easeInElastic(float x)
	{
		const float c4 = (2 * 3.14f) / 3;

		return x == 0.0f
			? 0
			: x == 1.0f
			? 1
			: -std::pow(2.0f, 10.0f * x - 10.0f) * std::sin((x * 10.0f - 10.75f) * c4);
	}

	float easeInOutQuint(float x)
	{
		return x < 0.5f ? 16.0f * x * x * x * x * x : 1 - std::pow(-2.0f * x + 2.0f, 5.0f) / 2.0f;
	}
}