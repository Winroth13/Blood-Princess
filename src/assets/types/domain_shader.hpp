#pragma once
#include "assets/asset.hpp"

struct ID3D11DomainShader;
struct ID3D11Device;

class DomainShader : public Asset
{
	ID3D11DomainShader *shader = nullptr;

public:
	DomainShader() = default;
	~DomainShader();

	bool Create(const std::string &data, ID3D11Device *device);

	ID3D11DomainShader *GetShader() { return this->shader; }
};