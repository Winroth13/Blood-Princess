#pragma once
#include "assets/asset.hpp"
#include <string>

struct ID3D11VertexShader;
struct ID3D11Device;

class VertexShader : public Asset
{
	ID3D11VertexShader *vertexShader = nullptr;
	std::string bytecode{};

public:
	VertexShader() = default;
	~VertexShader();

	bool Create(const std::string &data, ID3D11Device* device);

	ID3D11VertexShader *GetShader() { return this->vertexShader; }

	const std::string &GetByteCode() { return this->bytecode; }
};