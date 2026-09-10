#include "vertex_shader.hpp"
#include "rendering/render_utils.hpp"
#include "core/logging.hpp"

#include <d3d11.h>

VertexShader::~VertexShader()
{
	SafeRelease(this->vertexShader);
}

bool VertexShader::Create(const std::string &bytecode, ID3D11Device *device)
{
	LogInfo("Creating Vertex Shader");
	LogIndent();

	HRESULT hr = device->CreateVertexShader(
		bytecode.c_str(),
		bytecode.length(),
		nullptr,
		&this->vertexShader
	);

	if (FAILED(hr))
	{
		LogError("DirectX failed to create Vertex Shader");
		return false;
	}

	LogUnindent();

	return true;
}
