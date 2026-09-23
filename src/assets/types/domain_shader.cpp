#include "domain_shader.hpp"
#include "rendering/render_utils.hpp"
#include "core/logging.hpp"

#include <d3d11.h>

DomainShader::~DomainShader()
{
	SafeRelease(this->shader);
}

bool DomainShader::Create(const std::string &bytecode, ID3D11Device *device)
{
	LogInfo("Creating Domain Shader");
	LogIndent();

	HRESULT hr = device->CreateDomainShader(
		bytecode.c_str(),
		bytecode.length(),
		nullptr,
		&this->shader
	);

	if (FAILED(hr))
	{
		LogError("DirectX failed to create Domain Shader");
		return false;
	}

	LogUnindent();

	return true;
}
