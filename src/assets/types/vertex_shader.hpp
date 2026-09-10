#pragma once
#include "assets/asset.hpp"

struct ID3D11VertexShader;
struct ID3D11Device;

class VertexShader : public Asset
{
	ID3D11VertexShader *vertexShader = nullptr;

public:
	VertexShader() = default;
	~VertexShader();

	bool Create(const std::string &data, ID3D11Device* device);

	ID3D11VertexShader *GetShader() { return this->vertexShader; }
};