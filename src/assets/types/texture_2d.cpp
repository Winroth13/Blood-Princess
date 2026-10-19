#include "assets/types/texture_2d.hpp"

bool Texture2D::Create(
	char *data,
	const uint32_t width,
	const uint32_t height,
	const uint32_t channels,
	const std::string &name,
	bool isRGB,
	ID3D11Device *device,
	ID3D11DeviceContext *deviceContext
)
{
	return Texture::Create(
		&data, width, height,
		channels, name, "Texture",
		1, isRGB, device, deviceContext
	);
}