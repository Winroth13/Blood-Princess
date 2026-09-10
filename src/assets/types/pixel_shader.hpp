#pragma once
#include "assets/asset.hpp"

struct ID3D11PixelShader;
struct ID3D11Device;

class PixelShader : public Asset
{
	ID3D11PixelShader *pixelShader = nullptr;

public:
	PixelShader() = default;
	~PixelShader();

	bool Create(const std::string &data, ID3D11Device *device);

	ID3D11PixelShader *GetShader() { return this->pixelShader; }
};