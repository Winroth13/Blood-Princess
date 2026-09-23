#include "geometry_shader.hpp"
#include "rendering/render_utils.hpp"
#include "core/logging.hpp"

#include <d3d11.h>

GeometryShader::~GeometryShader()
{
	SafeRelease(this->shader);
}

bool GeometryShader::Create(const std::string &bytecode, ID3D11Device *device)
{
	LogInfo("Creating Geometry Shader");
	LogIndent();

	HRESULT hr = device->CreateGeometryShader(
		bytecode.c_str(),
		bytecode.length(),
		nullptr,
		&this->shader
	);

	if (FAILED(hr))
	{
		LogError("DirectX failed to create Geometry Shader");
		return false;
	}

	LogUnindent();

	return true;
}
