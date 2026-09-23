#pragma once
#include "assets/asset.hpp"

struct ID3D11GeometryShader;
struct ID3D11Device;

class GeometryShader : public Asset
{
	ID3D11GeometryShader *shader = nullptr;

public:
	GeometryShader() = default;
	~GeometryShader();

	bool Create(const std::string &data, ID3D11Device *device);

	ID3D11GeometryShader *GetShader() { return this->shader; }
};