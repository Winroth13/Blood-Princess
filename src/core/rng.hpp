#pragma once
#include <random>
#include <DirectXMath.h>

class RNG
{
private:
	
	static std::mt19937 generator;
	static unsigned int seed;

public:
	
	static void SetSeed(unsigned int newSeed);
	static unsigned int GetSeed();

	static float Float();
	static float Float(float min,float max); // Returns a value in [0, 1). Never exactly 1.


	static int Int(int min, int max); // Both ends inclusive: Int(1, 6) can return 6.


	static DirectX::XMFLOAT3 UnitVector();
	static DirectX::XMFLOAT3 InSphere(float radius);

};

