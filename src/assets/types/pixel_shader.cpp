#include "assets/types/pixel_shader.hpp"
#include "rendering/render_utils.hpp"
#include "core/logging.hpp"

#include <d3d11.h>

PixelShader::~PixelShader()
{
	SafeRelease(this->pixelShader);
}

bool PixelShader::Create(const std::string &bytecode, ID3D11Device *device)
{
	LogInfo("Creating Pixel Shader");
	LogIndent();

	HRESULT hr = device->CreatePixelShader(
		bytecode.c_str(),
		bytecode.length(),
		nullptr,
		&this->pixelShader
	);

	if (FAILED(hr))
	{
		LogError("DirectX failed to create Pixel Shader");
		return false;
	}

	LogUnindent();

	return true;
}
