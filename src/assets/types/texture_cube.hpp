#pragma once
#include "assets/asset.hpp"
#include "texture_2d.hpp"

struct ID3D11Device;

class TextureCube : public Asset
{

	ID3D11ShaderResourceView *shaderResourceView = nullptr;

public:
	TextureCube();
	~TextureCube();

	//bool Create(ID3D11Device *device);

	ID3D11ShaderResourceView *GetShaderResourceView() const { return this->shaderResourceView; }
};