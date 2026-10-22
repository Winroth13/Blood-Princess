#pragma once

#include <cmath>

namespace Math
{
	float Lerp(float a, float b, float x);
	float easeOutElastic(float x);
	float easeInElastic(float x);
	float easeInOutQuint(float x);
	float easeOutQuint(float x);
}