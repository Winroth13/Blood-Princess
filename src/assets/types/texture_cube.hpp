#pragma once

#include "assets/texture.hpp"

class TextureCube : public Texture
{
public:
	bool Create(
		char **data,
		const uint32_t width,
		const uint32_t height,
		const uint32_t channels,
		const std::string &name,
		bool isRGB,
		ID3D11Device *device,
		ID3D11DeviceContext *deviceContext
	);
};