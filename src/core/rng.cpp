#include "rng.hpp"
#include <numbers>
#include <cmath>


//unsigned int RNG::seed = std::random_device{}();
unsigned int RNG::seed = 42; // Set seed under development
std::mt19937 RNG::generator(RNG::seed);


void RNG::SetSeed(unsigned int newSeed)
{
	seed = newSeed;
	generator.seed(newSeed);
}

unsigned int RNG::GetSeed()
{
	return seed;
}


float RNG::Float()
{
	std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
	return distribution(generator);
}

float RNG::Float(float min, float max)
{
	return (min + Float() * (max - min));
}

int RNG::Int(int min, int max)
{
	std::uniform_int_distribution distribution(min, max);
	return distribution(generator);
}

DirectX::XMFLOAT3 RNG::UnitVector()
{
	float y = Float(-1.0f, 1.0f);
	float angle = Float(0.0f, 2 * std::numbers::pi_v<float>);

	float radius = sqrt(1 - y * y);

	float x = radius * cos(angle);
	float z = radius * sin(angle);

	return DirectX::XMFLOAT3(x, y, z);

}

DirectX::XMFLOAT3 RNG::InSphere(float radius)
{
	while (true)
	{
		const float x = Float(-1.0f, 1.0f);
		const float y = Float(-1.0f, 1.0f);
		const float z = Float(-1.0f, 1.0f);

		if (x * x + y * y + z * z <= 1.0f)
		{
			return DirectX::XMFLOAT3(x * radius, y * radius, z * radius);
		}
	}
}