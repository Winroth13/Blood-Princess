#include "core/math.hpp"

#include "cmath"

namespace Math
{
	float Lerp(float a, float b, float x)
	{
		return a + x * (b - a);
	}

	XMFLOAT4 Lerp(XMFLOAT4 a, XMFLOAT4 b, float x)
	{
		return {
			Lerp(a.x, b.x, x),
			Lerp(a.y, b.y, x),
			Lerp(a.z, b.z, x),
			Lerp(a.w, b.w, x),
		};
	}

	float easeInElastic(float x)
	{
		const float c4 = (2 * 3.14f) / 3;

		return x == 0.0f
			? 0
			: x == 1.0f
			? 1
			: -std::pow(2.0f, 10.0f * x - 10.0f) * std::sin((x * 10.0f - 10.75f) * c4);
	}

	float easeOutElastic(float x)
	{
		const float c4 = (2.0f * 3.14) / 3.0f;

		return x == 0.0f
			? 0.0f
			: x == 1.0f
			? 1.0f
			: std::powf(2.0f, -10.0f * x) * std::sin((x * 10.0f - 0.75f) * c4) + 1.0f;
	}

	float easeInOutQuint(float x)
	{
		return x < 0.5f ? 16.0f * x * x * x * x * x : 1 - std::pow(-2.0f * x + 2.0f, 5.0f) / 2.0f;
	}

	float easeOutQuint(float x)
	{
		return 1.0f - std::pow(1.0f - x, 5.0f);
	}
}