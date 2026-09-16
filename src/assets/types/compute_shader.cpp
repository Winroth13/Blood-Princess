#include "compute_shader.hpp"
#include "rendering/render_utils.hpp"
#include "core/logging.hpp"

#include <d3d11.h>

ComputeShader::~ComputeShader()
{
	SafeRelease(this->computeShader);
}

bool ComputeShader::Create(const std::string &bytecode, ID3D11Device *device)
{
	LogInfo("Creating Compute Shader");
	LogIndent();

	HRESULT hr = device->CreateComputeShader(
		bytecode.c_str(),
		bytecode.length(),
		nullptr,
		&this->computeShader
	);

	if (FAILED(hr))
	{
		LogError("DirectX failed to create Compute Shader");
		return false;
	}

	LogUnindent();

	return true;
}
