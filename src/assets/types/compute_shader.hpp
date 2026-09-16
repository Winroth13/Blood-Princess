#pragma once
#include "assets/asset.hpp"

struct ID3D11ComputeShader;
struct ID3D11Device;

class ComputeShader : public Asset
{
	ID3D11ComputeShader *computeShader = nullptr;

public:
	ComputeShader() = default;
	~ComputeShader();

	bool Create(const std::string &data, ID3D11Device *device);

	ID3D11ComputeShader *GetShader() { return this->computeShader; }
};